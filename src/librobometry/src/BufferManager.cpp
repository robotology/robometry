/*
 * Copyright (C) 2006-2022 Istituto Italiano di Tecnologia (IIT)
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the
 * BSD-3-Clause license. See the accompanying LICENSE file for details.
 */

#include <robometry/BufferManager.h>

robometry::BufferInfo::BufferInfo(const BufferInfo &other)
    : m_buffer(other.m_buffer),
      m_dimensions(other.m_dimensions),
      m_dimensions_factorial(other.m_dimensions_factorial),
      m_type_name(other.m_type_name),
      m_elements_names(other.m_elements_names),
      m_convert_to_matioCpp(other.m_convert_to_matioCpp){
}

robometry::BufferInfo::BufferInfo(BufferInfo &&other)
    : m_buffer(std::move(other.m_buffer)),
      m_dimensions(std::move(other.m_dimensions)),
      m_dimensions_factorial(std::move(other.m_dimensions_factorial)),
      m_type_name(std::move(other.m_type_name)),
      m_elements_names(std::move(other.m_elements_names)),
      m_convert_to_matioCpp(std::move(other.m_convert_to_matioCpp)){
}

robometry::BufferManager::BufferManager() {
    m_tree = std::make_shared<TreeNode<BufferInfo>>();
}

robometry::BufferManager::BufferManager(const BufferConfig &_bufferConfig) {
    m_tree = std::make_shared<TreeNode<BufferInfo>>();
    bool ok = configure(_bufferConfig);
    assert(ok);
    // For avoiding warnings in Release
    ROBOMETRY_UNUSED(ok)
}

robometry::BufferManager::~BufferManager() {
    if (m_save_thread.joinable()) {
        // This additional brackets are needed for make unique_lock out of scope before the join
        {
            std::unique_lock<std::mutex> lk_cv(m_mutex_cv);
            m_should_stop_thread = true;
            m_cv.notify_one(); // Wake up the thread in order to close it
        }
        m_save_thread.join();
    }
    if (m_bufferConfig.auto_save) {
        std::string fileName;
        saveToFile(fileName);
        if (m_saveCallback)
        {
            m_saveCallback(fileName, SaveCallbackSaveMethod::last_call);
        }
    }
}

bool robometry::BufferManager::enablePeriodicSave(double _save_period) {
    // If it is not joinable means it is not running
    if (!m_save_thread.joinable()) {
        m_bufferConfig.save_periodically = true;
        m_bufferConfig.save_period = _save_period;
        m_save_thread = std::thread(&BufferManager::periodicSave, this);
        return true;
    }
    return false;
}

bool robometry::BufferManager::configure(const BufferConfig &_bufferConfig) {
    bool ok{ true };
    if (_bufferConfig.filename == "")
    {
        std::cout << "The filename cannot be empty." << std::endl;
        return false;
    }
    set_capacity(_bufferConfig.n_samples);
    m_bufferConfig = _bufferConfig;
    if (!_bufferConfig.channels.empty()) {
        ok = ok && addChannels(_bufferConfig.channels);
    }
    if (ok && _bufferConfig.save_periodically) {
        ok = ok && enablePeriodicSave(_bufferConfig.save_period);
    }
    populateDescriptionCellArray();
    if (!m_bufferConfig.path.empty() && !yarp_telemetry_fs::exists(m_bufferConfig.path)) {
        std::error_code ec;
        auto dir_created = yarp_telemetry_fs::create_directory(m_bufferConfig.path, ec);
        if (!dir_created) {
            std::cout << m_bufferConfig.path << " does not exists, and it was not possible to create it." << std::endl;
            return false;
        }
    }
    // TODO ROLL BACK IN CASE OF FAILURE
    return ok;
}

robometry::BufferConfig robometry::BufferManager::getBufferConfig() const {
    return m_bufferConfig;
}

void robometry::BufferManager::setFileName(const std::string &filename) {
    m_bufferConfig.filename = filename;
    return;
}

void robometry::BufferManager::setDefaultPath(const std::string &path) {
    m_bufferConfig.path = path;
    return;
}

void robometry::BufferManager::enableCompression(bool enable_compression) {
    m_bufferConfig.enable_compression = enable_compression;
    return;
}

void robometry::BufferManager::setDescriptionList(const std::vector<std::string> &description_list) {
    m_bufferConfig.description_list = description_list;
    populateDescriptionCellArray();
    return;
}

void robometry::BufferManager::resize(size_t new_size) {
    this->resize(new_size, m_tree);
    m_bufferConfig.n_samples = new_size;
    return;
}

void robometry::BufferManager::set_capacity(size_t new_size) {
    this->set_capacity(new_size, m_tree);
    m_bufferConfig.n_samples = new_size;
    return;
}

bool robometry::BufferManager::addChannel(const ChannelInfo &channel) {
    auto buffInfo = std::make_shared<BufferInfo>();
    buffInfo->m_buffer = Buffer(m_bufferConfig.n_samples);
    buffInfo->m_dimensions = channel.dimensions;

    buffInfo->m_dimensions_factorial = std::accumulate(channel.dimensions.begin(),
                                                       channel.dimensions.end(),
                                                       1,
                                                       std::multiplies<>());

    buffInfo->m_elements_names = channel.elements_names;

    const bool ok = addLeaf(channel.name, buffInfo, m_tree);
    if(ok) {
        m_bufferConfig.channels.push_back(channel);
    }
    return ok;
}

bool robometry::BufferManager::addChannels(const std::vector<ChannelInfo> &channels) {
    if (channels.empty()) {
        return false;
    }
    bool ret{ true };
    for (const auto& c : channels) {
        ret = ret && addChannel(c);
    }
    return ret;
}

bool robometry::BufferManager::saveToFile(bool flush_all) {
    std::string dummy_file_name;
    return saveToFile(dummy_file_name, flush_all);
}

bool robometry::BufferManager::saveToFile(std::string &file_name_path, bool flush_all) {

    // now we initialize the proto-timeseries structure
    std::vector<matioCpp::Variable> signalsVect, descrListVect;
    // and the matioCpp struct for these signals
    // Add the description
    if (m_description_cell_array.isValid()) {
        signalsVect.emplace_back(m_description_cell_array);
    }

    signalsVect.emplace_back(matioCpp::String("yarp_robot_name", m_bufferConfig.yarp_robot_name));

    // we have to force the flush.
    flush_all = flush_all || (m_bufferConfig.data_threshold > m_bufferConfig.n_samples);
    for (auto& [node_name, node] : m_tree->getChildren()) {

        // now we create the vector that stores different signals (in case we had more than one)
        signalsVect.emplace_back(this->createTreeStruct(node_name, node, flush_all));
    }

    // This means that no variables are logged, we have only the description_list (if set) and the yarp_robot_name
    if (signalsVect.size() == static_cast<size_t>(1 + m_description_cell_array.isValid())) {
        return false;
    }

    matioCpp::Struct timeSeries(m_bufferConfig.filename, signalsVect);
    // and finally we write the file
    // since we might save several files, we need to index them
    file_name_path = m_bufferConfig.filename + "_" + this->fileIndex();
    if (!m_bufferConfig.path.empty()) {
        file_name_path = m_bufferConfig.path + file_name_path;
    }
    std::string new_file = file_name_path + ".mat";
    assert(!matioCpp::File::Exists(new_file) && "A file with the same name already exists.");
    matioCpp::File file = matioCpp::File::Create(new_file, m_bufferConfig.mat_file_version);
    assert(file.isOpen() && "Failed to open the specified file.");
    bool ok = file.write(timeSeries, m_bufferConfig.enable_compression ? matioCpp::Compression::zlib : matioCpp::Compression::None);

    if (!ok)
    {
        std::cout << "An error occurred while saving the data to the file." << std::endl;
    }

    return ok;
}

bool robometry::BufferManager::setNowFunction(std::function<double ()> now)
{
    if (now == nullptr) {
        std::cout << "Not valid clock function." << std::endl;
        return false;
    }

    m_nowFunction = now;
    return true;
}

bool robometry::BufferManager::setSaveCallback(std::function<bool (const std::string &, const SaveCallbackSaveMethod &)> saveCallback)
{
    if (saveCallback == nullptr) {
        std::cout << "Not valid saveCallback function." << std::endl;
        return false;
    }

    m_saveCallback = saveCallback;;
    return true;
}

double robometry::BufferManager::DefaultClock() {
    return std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count();
}

void robometry::BufferManager::periodicSave()
{
    std::unique_lock<std::mutex> lk_cv(m_mutex_cv);

    auto timeout =  std::chrono::duration<double>(m_bufferConfig.save_period);
    // For avoiding spurious wake up, the lambda check that the threads wake up only if we are trying to close
    // (additionally to the timeout expiration)
    while (!(m_cv.wait_for(lk_cv, timeout, [this](){return m_should_stop_thread;})))
    {
        if (!m_tree->empty()) // if there are channels
        {
            std::string fileName;
            saveToFile(fileName, false);
            if (m_saveCallback)
            {
                m_saveCallback(fileName, SaveCallbackSaveMethod::periodic);
            }
        }
    }
}

matioCpp::Struct robometry::BufferManager::createTreeStruct(const std::string &node_name, std::shared_ptr<TreeNode<BufferInfo> > tree_node, bool flush_all) {
    const auto& children = tree_node->getChildren();
    if (children.size() == 0) {
        return createElementStruct(node_name, tree_node->getValue(), flush_all);
    }

    matioCpp::Struct tmp(node_name);
    for (const auto& [child_name, child] : tree_node->getChildren()) {
        tmp.setField(this->createTreeStruct(child_name, child, flush_all));
    }

    return tmp;
}

matioCpp::Struct robometry::BufferManager::createElementStruct(const std::string &var_name, std::shared_ptr<BufferInfo> buffInfo, bool flush_all) const {

    assert(buffInfo);

    std::scoped_lock<std::mutex> lock{ buffInfo->m_buff_mutex };
    if (buffInfo->m_buffer.empty()) {
        std::cout << var_name << " does not contain data, skipping" << std::endl;
        return matioCpp::Struct(var_name);
    }

    if (!flush_all && buffInfo->m_buffer.size() < m_bufferConfig.data_threshold) {
        std::cout << var_name << " does not contain enought data, skipping" << std::endl;
        return matioCpp::Struct(var_name);
    }

    // the number of timesteps is the size of our collection
    auto num_timesteps = buffInfo->m_buffer.size();

    assert(buffInfo->m_convert_to_matioCpp);
    // We concatenate all the data of the buffer into a single variable
    matioCpp::Variable data = buffInfo->m_convert_to_matioCpp("data");

    //We construct the timestamp vector
    matioCpp::Vector<double> timestamps("timestamps", num_timesteps);
    size_t i = 0;
    for (auto& _cell : buffInfo->m_buffer) {
        timestamps[i] = _cell.m_ts;
        ++i;
    }
    assert(i == buffInfo->m_buffer.size());

    //Clear the buffer, we don't need it anymore
    buffInfo->m_buffer.clear();

    //Create the set of variables to be used in the output struct
    std::vector<matioCpp::Variable> test_data;

    // now we create the vector for the dimensions
    dimensions_t fullDimensions = buffInfo->m_dimensions;
    fullDimensions.push_back(num_timesteps);

    test_data.emplace_back(data); // Data

    test_data.emplace_back(matioCpp::make_variable("dimensions", fullDimensions)); // dimensions vector
    test_data.emplace_back(matioCpp::make_variable("elements_names", buffInfo->m_elements_names)); // elements names

    test_data.emplace_back(matioCpp::String("name", var_name)); // name of the signal
    test_data.emplace_back(timestamps);

    return matioCpp::Struct(var_name, test_data);
}

std::string robometry::BufferManager::fileIndex() const {
    if (m_bufferConfig.file_indexing == "time_since_epoch") {
        return std::to_string(m_nowFunction());
    }
    std::time_t t = std::time(nullptr);
    std::tm tm = *std::localtime(&t);
    std::stringstream time;
    time << std::put_time(&tm, m_bufferConfig.file_indexing.c_str());
    return time.str();
}

void robometry::BufferManager::populateDescriptionCellArray() {
    if (m_bufferConfig.description_list.empty())
        return;
    std::vector<matioCpp::Variable> descrListVect;
    for (const auto& str : m_bufferConfig.description_list) {
        descrListVect.emplace_back(matioCpp::String("useless_name",str));
    }
    matioCpp::CellArray description_list("description_list", { m_bufferConfig.description_list.size(), 1 }, descrListVect);
    m_description_cell_array = description_list;
}

void robometry::BufferManager::resize(size_t new_size, std::shared_ptr<TreeNode<BufferInfo> > node) {

    // resize the variable
    auto variable = node->getValue();
    if (variable != nullptr) {
        variable->m_buffer.resize(new_size);
    }

    for (auto& [var_name, child] : node->getChildren()) {
        this->resize(new_size, child);
    }
}

void robometry::BufferManager::set_capacity(size_t new_size, std::shared_ptr<TreeNode<BufferInfo> > node) {

    // resize the variable
    auto variable = node->getValue();
    if (variable != nullptr) {
        variable->m_buffer.set_capacity(new_size);
    }

    for (auto& [var_name, child] : node->getChildren()) {
        this->set_capacity(new_size, child);
    }
}
