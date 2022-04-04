/*
 * Copyright (C) 2006-2021 Istituto Italiano di Tecnologia (IIT)
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the
 * BSD-3-Clause license. See the accompanying LICENSE file for details.
 */

#ifndef YARP_TELEMETRY_BUFFER_MANAGER_H
#define YARP_TELEMETRY_BUFFER_MANAGER_H

#include <initializer_list>
#include <yarp/telemetry/experimental/Buffer.h>
#include <yarp/telemetry/experimental/BufferConfig.h>
#include <yarp/telemetry/experimental/TreeNode.h>

#include <boost/core/demangle.hpp>
#include <matioCpp/matioCpp.h>

#include <unordered_map>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <assert.h>
#include <functional>
#include <chrono>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <iomanip>
#include <stdexcept>
#include <typeinfo>


#ifndef __has_include
  static_assert(false, "__has_include not supported");
#else
#  if __has_include(<filesystem>)
#    include <filesystem>
     namespace yarp_telemetry_fs = std::filesystem;
#  elif __has_include(<experimental/filesystem>)
#    include <experimental/filesystem>
     namespace yarp_telemetry_fs = std::experimental::filesystem;
#  else
     static_assert(false, "Neither <filesystem> nor <experimental/filesystem> headers are present in the system, but they are required");
#  endif
#endif


namespace yarp::telemetry::experimental {

/**
 * @brief Get the type name as string
 */
template<typename T>
static std::string getTypeName(const T& someInput)
{
    return boost::core::demangle(typeid(someInput).name());
}

/**
 * @brief Get the type name as string
 */
template<typename T>
static std::string getTypeName()
{
    return boost::core::demangle(typeid(T).name());
}

// matiomatioCppCanConcatenate<T>::value is true when T has the T::value_type memeber. If this is true, then we check
// if T is either an Element, a Vector (but not a String), or a MultidimensionalArray
template <typename T, typename = void, typename = void>
struct matioCppCanConcatenate : std::false_type {};

template<typename T>
struct matioCppCanConcatenate<T,
                              typename std::enable_if_t<matioCpp::SpanUtils::has_type_member<T>::value>,
                              typename std::enable_if_t<(std::is_same_v<T, matioCpp::Element<typename T::value_type>> ||
                                                         (std::is_same_v<T, matioCpp::Vector<typename T::value_type>> &&
                                                          !std::is_same_v<T, matioCpp::String>) ||
                                                         std::is_same_v<T, matioCpp::MultiDimensionalArray<typename T::value_type>>)>>
        : std::true_type {};



/**
* @brief Class that aggregates the yarp::telemetry::experimental::Buffer and some other
* info(e.g. dimensions) used by the yarp::telemetry::experimental::BufferManager
*
*/
struct BufferInfo {
    inline static std::string type_name_not_set_tag = "type_name_not_set";
    Buffer m_buffer;
    std::mutex m_buff_mutex;
    dimensions_t m_dimensions;
    size_t m_dimensions_factorial{0};
    std::string m_type_name{type_name_not_set_tag};
    elements_names_t m_elements_names;
    std::function<matioCpp::Variable(const std::string&)> m_convert_to_matioCpp;

    BufferInfo() = default;
    BufferInfo(const BufferInfo& other)
        : m_buffer(other.m_buffer),
          m_dimensions(other.m_dimensions),
          m_dimensions_factorial(other.m_dimensions_factorial),
          m_type_name(other.m_type_name),
          m_elements_names(other.m_elements_names),
          m_convert_to_matioCpp(other.m_convert_to_matioCpp){
    }

    BufferInfo(BufferInfo&& other)
        : m_buffer(std::move(other.m_buffer)),
          m_dimensions(std::move(other.m_dimensions)),
          m_dimensions_factorial(std::move(other.m_dimensions_factorial)),
          m_type_name(std::move(other.m_type_name)),
          m_elements_names(std::move(other.m_elements_names)),
          m_convert_to_matioCpp(std::move(other.m_convert_to_matioCpp)){
    }

    template<typename T>
    void createMatioCppConvertFunction()
    {
        static_assert(matioCpp::is_make_variable_callable<T>::value, "The selected type cannot be used with matioCpp.");

        if (m_convert_to_matioCpp)
        {
            return;
        }

        using matioCppType = typename matioCpp::make_variable_output<T>::type;
        m_convert_to_matioCpp = [this](const std::string& name)
        {
            size_t num_instants = this->m_buffer.size();

            //if the input data is numeric, then we concatenate on the last dimension
            if constexpr (matioCppCanConcatenate<matioCppType>::value)
            {
                using elementType = typename matioCppType::value_type;

                dimensions_t fullDimensions = this->m_dimensions;
                fullDimensions.push_back(num_instants);

                matioCpp::MultiDimensionalArray<elementType> outputVariable(name, fullDimensions);

                size_t i = 0;
                for (auto& _cell : this->m_buffer) {
                    const T& cellCasted = std::any_cast<T>(_cell.m_datum);

                    matioCpp::Span<const elementType> matioCppSpan;

                    matioCppType matioCppVariable;

                    //We convert the cell to a matioCpp variable only if we are not able to create a Span.
                    //In this way we avoid duplicating memory
                    if constexpr (matioCpp::SpanUtils::is_make_span_callable<const T&>::value)
                    {
                        matioCppSpan = matioCpp::make_span(cellCasted);
                    }
                    else
                    {
                        matioCppVariable = matioCpp::make_variable("element", cellCasted);

                        matioCppSpan = matioCppVariable.toSpan();
                    }

                    size_t startIndex = this->m_dimensions_factorial * i; //We concatenate on the last dimension. Suppose that the channel stores matrices of size 3x2.
                                                                          //The output variable is a 3x2xn matrix, where n is the number of elements in the buffer.
                                                                          //If we consider the output buffer as a linear vector, the element at position i starts from location 6*i
                                                                          //and ends at 6*(i+1)

                    for (size_t i = 0; i < std::min(static_cast<size_t>(matioCppSpan.size()), this->m_dimensions_factorial); ++i)
                    {
                        outputVariable[startIndex + i] = matioCppSpan[i]; //we copy the new element in the corresponding position inside the variable
                    }
                    ++i;
                }
                return outputVariable;
            }
            else if constexpr(std::is_same_v<matioCppType, matioCpp::Struct>) //if the input is a struct, we use a struct array
            {
                matioCpp::StructArray outputVariable(name, {num_instants, 1});

                size_t i = 0;
                for (auto& _cell : this->m_buffer) {
                    matioCpp::Struct element = matioCpp::make_variable("element", std::any_cast<T>(_cell.m_datum));

                    if (i == 0)
                    {
                        outputVariable.addFields(element.fields());
                    }

                    outputVariable.setElement(i, element);
                    ++i;
                }
                return outputVariable;
            }
            else //otherwise we use a cell array
            {
                matioCpp::CellArray outputVariable(name, {num_instants, 1});

                size_t i = 0;
                for (auto& _cell : this->m_buffer) {
                    outputVariable.setElement(i, matioCpp::make_variable("element", std::any_cast<T>(_cell.m_datum)));
                    ++i;
                }
                return outputVariable;
            }
        };
    }


};
/**
 * @brief Class that manages the buffers associated to the channels of the telemetry.
 * Each BufferManager can handle one type of data, the number of samples is defined in the configuration and
 * it is the same for every channel.
 * On the other hand the data inside the channels can have different dimensionality(e.g. 1x1, 2x3 etc).
 * It contains utilities for saving the data of the channels in mat files, and to save/read the configuration
 * to/from a json file.
 *
 */
template <typename DefaultVectorType = void>
class BufferManager {

public:
    /**
     * @brief Construct an empty BufferManager object.
     * For being used it has to be configured afterwards.
     *
     */
    BufferManager() {
        m_tree = std::make_shared<TreeNode<BufferInfo>>();
    }

    /**
     * @brief Construct a new BufferManager object, configuring it via
     * the yarp::telemetry::experimental::BufferConfig.
     *
     * @param[in] _bufferConfig The struct containing the configuration for the BufferManager.
     */
    BufferManager(const BufferConfig& _bufferConfig) {
        m_tree = std::make_shared<TreeNode<BufferInfo>>();
        bool ok = configure(_bufferConfig);
        assert(ok);
    }

    /**
     * @brief Destroy the BufferManager object.
     * If auto_save is enabled, it saves to file the remaining data in the buffer.
     *
     */
    ~BufferManager() {
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
            saveToFile();
        }
    }

    /**
     * @brief Enable the save thread with _save_period seconds of period.
     * If the thread has been started yet in the configuration through
     * BufferConfing, it skips it.
     *
     * @param[in] _save_period The period in seconds of the save thread.
     * @return true on success, false otherwise.
     */
    bool enablePeriodicSave(double _save_period) {
        // If it is not joinable means it is not running
        if (!m_save_thread.joinable()) {
            m_bufferConfig.save_periodically = true;
            m_bufferConfig.save_period = _save_period;
            m_save_thread = std::thread(&BufferManager::periodicSave, this);
            return true;
        }
        return false;
    }

    /**
     * @brief Configure the BufferManager through a BufferConfig object.
     *
     * @param[in] _bufferConfig The struct containing the configuration parameters.
     * @return true on success, false otherwise.
     */
    bool configure(const BufferConfig& _bufferConfig) {
        bool ok{ true };
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

    /**
     * @brief Get the BufferConfig object representing the actual configuration.
     *
     * @return The BufferConfig object.
     */
    BufferConfig getBufferConfig() const {
        return m_bufferConfig;
    }
    /**
     * @brief Set the file name that will be created by the BufferManager.
     *
     * @param[in] filename The file name to be set.
     */
    void setFileName(const std::string& filename) {
        m_bufferConfig.filename = filename;
        return;
    }

    /**
     * @brief Set the path where the files will be saved.
     *
     * @param[in] path The path to be set.
     */
    void setDefaultPath(const std::string& path) {
        m_bufferConfig.path = path;
        return;
    }

    /**
    * @brief Enable the zlib compression.
    *
    * @param[in] flag for enabling/disabling compression.
    */
    void enableCompression(bool enable_compression) {
        m_bufferConfig.enable_compression = enable_compression;
        return;
    }

    /**
     * @brief Set the description list that will be saved in all the files.
     *
     * @param[in] description The description to be set.
     */
    void setDescriptionList(const std::vector<std::string>& description_list) {
        m_bufferConfig.description_list = description_list;
        populateDescriptionCellArray();
        return;
    }

    /**
     * @brief Resize the Buffer/s.
     *
     * @param[in] new_size The new size to be resized to.
     */
    void resize(size_t new_size) {
        this->resize(new_size, m_tree);
        m_bufferConfig.n_samples = new_size;
        return;
    }

    /**
     * @brief Set the capacity of Buffer/s.
     *
     * @param[in] new_size The new size.
     */
    void set_capacity(size_t new_size) {
        this->set_capacity(new_size, m_tree);
        m_bufferConfig.n_samples = new_size;
        return;
    }

    /**
     * @brief Add a channel(variable) to the BufferManager.
     * The channels have to be unique in the BufferManager.
     *
     * @param[in] channel Pair representing the channel to be added.
     * @return true on success, false otherwise.
     */
    bool addChannel(const ChannelInfo& channel) {
        auto buffInfo = std::make_shared<BufferInfo>();
        buffInfo->m_buffer = Buffer(m_bufferConfig.n_samples);
        buffInfo->m_dimensions = channel.dimensions;

        buffInfo->m_dimensions_factorial = std::accumulate(channel.dimensions.begin(),
                                                           channel.dimensions.end(),
                                                           1,
                                                           std::multiplies<>());

        if constexpr (!std::is_same_v<DefaultVectorType, void>)
        {
            buffInfo->m_type_name = getTypeName<std::vector<DefaultVectorType>>();
        }

        buffInfo->m_elements_names = channel.elements_names;

        const bool ok = addLeaf(channel.name, buffInfo, m_tree);
        if(ok) {
            m_bufferConfig.channels.push_back(channel);
        }
        return ok;
    }

    /**
     * @brief Add a list of channels(variables) to the BufferManager.
     * The channels have to be unique in the BufferManager.
     *
     * @param[in] channels List of pair representing the channels to be added.
     * @return true on success, false otherwise.
     */
    bool addChannels(const std::vector<ChannelInfo>& channels) {
        if (channels.empty()) {
            return false;
        }
        bool ret{ true };
        for (const auto& c : channels) {
            ret = ret && addChannel(c);
        }
        return ret;
    }

    /**
     * @brief Push a new element in the var_name channel.
     * The var_name channels must exist, otherwise an exception is thrown.
     *
     * @param[in] elem The element to be pushed(via copy) in the channel.
     * @param[in] ts The timestamp of the element to be pushed.
     * @param[in] var_name The name of the channel.
     */
    template<typename T>
    inline void push_back(matioCpp::Span<const T> elem, double ts, const std::string& var_name)
    {
        push_back(std::vector<T>(elem.begin(), elem.end()), ts, var_name);
    }

    /**
     * @brief Push a new element in the var_name channel.
     * The var_name channels must exist, otherwise an exception is thrown.
     *
     * @param[in] elem The element to be pushed(via copy) in the channel.
     * @param[in] ts The timestamp of the element to be pushed.
     * @param[in] var_name The name of the channel.
     */
    template<typename T>
    inline void push_back(const std::initializer_list<T>& elem, double ts, const std::string& var_name)
    {
        push_back(std::vector<T>(elem.begin(), elem.end()), ts, var_name);
    }

    /**
     * @brief Push a new element in the var_name channel.
     * The var_name channels must exist, otherwise an exception is thrown.
     *
     * @param[in] elem The element to be pushed(via copy) in the channel.
     * @param[in] var_name The name of the channel.
     */
    template<typename T>
    inline void push_back(matioCpp::Span<const T> elem, const std::string& var_name)
    {
        push_back(elem, m_nowFunction(), var_name);
    }

    /**
     * @brief Push a new element in the var_name channel.
     * The var_name channels must exist, otherwise an exception is thrown.
     *
     * @param[in] elem The element to be pushed(via copy) in the channel.
     * @param[in] var_name The name of the channel.
     */
    template<typename T>
    inline void push_back(const std::initializer_list<T>& elem, const std::string& var_name)
    {
        push_back(elem, m_nowFunction(), var_name);
    }

    /**
     * @brief Push a new element in the var_name channel.
     * The var_name channels must exist, otherwise an exception is thrown.
     *
     * @param[in] elem The element to be pushed in the channel.
     * @param[in] ts The timestamp of the element to be pushed.
     * @param[in] var_name The name of the channel.
     */
    template<typename T>
    inline void push_back(const T& elem, double ts, const std::string& var_name)
    {
        auto leaf = getLeaf(var_name, m_tree).lock();
        if (leaf == nullptr)
        {
            throw std::invalid_argument("The channel " + var_name + " does not exist.");
        }
        auto bufferInfo = leaf->getValue();
        assert(bufferInfo != nullptr);

        bool typename_set = bufferInfo->m_type_name != BufferInfo::type_name_not_set_tag;

        if (typename_set && (bufferInfo->m_type_name != getTypeName<T>()))
        {
            std::cout << "Cannot push to the channel " << var_name
                      << ". Expected type: " << bufferInfo->m_type_name
                      << ". Input type: " << getTypeName<T>() <<std::endl;
            return;
        }

        std::scoped_lock<std::mutex> lock{ bufferInfo->m_buff_mutex };

        if (!typename_set)
        {
            bufferInfo->m_type_name = getTypeName<T>();
        }

        //Create the saving functions if they were not present already
        bufferInfo->createMatioCppConvertFunction<T>();

        bufferInfo->m_buffer.push_back({ts, elem});
    }

    /**
     * @brief Push a new element in the var_name channel.
     * The var_name channels must exist, otherwise an exception is thrown.
     *
     * @param[in] elem The element to be pushed in the channel.
     * @param[in] var_name The name of the channel.
     */
    template<typename T>
    inline void push_back(const T& elem, const std::string& var_name)
    {
        push_back(elem, m_nowFunction(), var_name);
    }

    /**
     * @brief Save the content of all the channels into a file.
     * If flush_all is set to false, it saves only the content of the channels that
     * have a number of samples greater than the yarp::telemetry::experimental::BufferConfig::data_threshold.
     * If yarp::telemetry::experimental::BufferConfig::data_threshold is greater than yarp::telemetry::experimental::BufferConfig::n_samples
     * this check is skipped.
     *
     * @param[in] flush_all Flag for forcing the save of whatever is contained in the channels.
     * @return true on success, false otherwise.
     */
    bool saveToFile(bool flush_all=true) {

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
        if (signalsVect.empty()) {
            return false;
        }
        // This means that no variables are logged, we have only the description_list and the yarp_robot_name
        else if (signalsVect.size() == 2 && m_description_cell_array.isValid()) {
            return false;
        }
        matioCpp::Struct timeSeries(m_bufferConfig.filename, signalsVect);
        // and finally we write the file
        // since we might save several files, we need to index them
        std::string new_file = m_bufferConfig.filename + "_" + this->fileIndex() + ".mat";
        if (!m_bufferConfig.path.empty()) {
            new_file = m_bufferConfig.path + new_file;
        }
        matioCpp::File file = matioCpp::File::Create(new_file, m_bufferConfig.mat_file_version);
        return file.write(timeSeries, m_bufferConfig.enable_compression ? matioCpp::Compression::zlib : matioCpp::Compression::None);
    }

    /**
     * @brief Set the now function, by default is std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count().
     * @param[in] now The now function
     * @return true on success, false otherwise.
     */
    bool setNowFunction(std::function<double(void)> now)
    {
        if (now == nullptr) {
            std::cout << "Not valid clock function." << std::endl;
            return false;
        }

        m_nowFunction = now;
        return true;
    }

private:
    static double DefaultClock() {
        return std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count();
    }

    void periodicSave()
    {
        std::unique_lock<std::mutex> lk_cv(m_mutex_cv);

        auto timeout =  std::chrono::duration<double>(m_bufferConfig.save_period);
        // For avoiding spurious wake up, the lambda check that the threads wake up only if we are trying to close
        // (additionally to the timeout expiration)
        while (!(m_cv.wait_for(lk_cv, timeout, [this](){return m_should_stop_thread;})))
        {
            if (!m_tree->empty()) // if there are channels
            {
                saveToFile(false);
            }
        }
    }

    matioCpp::Struct createTreeStruct(const std::string& node_name,
                                      std::shared_ptr<TreeNode<BufferInfo>> tree_node,
                                      bool flush_all) {
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

    matioCpp::Struct createElementStruct(const std::string& var_name,
                                         std::shared_ptr<BufferInfo> buffInfo,
                                         bool flush_all) const {

        assert(buffInfo);

        std::scoped_lock<std::mutex> lock{ buffInfo->m_buff_mutex };
        if (buffInfo->m_buffer.empty()) {
            std::cout << var_name << " does not contain data, skipping" << std::endl;
            return matioCpp::Struct();
        }

        if (!flush_all && buffInfo->m_buffer.size() < m_bufferConfig.data_threshold) {
            std::cout << var_name << " does not contain enought data, skipping" << std::endl;
            return matioCpp::Struct();
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

    /**
    * This is an helper function that can be used to generate the file indexing accordingly to the
    * content of `m_bufferConfig.file_indexing`
    * @return a string containing the index
    */
    std::string fileIndex() const {
        if (m_bufferConfig.file_indexing == "time_since_epoch") {
            return std::to_string(m_nowFunction());
        }
        std::time_t t = std::time(nullptr);
        std::tm tm = *std::localtime(&t);
        std::stringstream time;
        time << std::put_time(&tm, m_bufferConfig.file_indexing.c_str());
        return time.str();
    }

    /**
    * This is an helper function that will be disappear the day matio-cpp
    * will support the std::vector<std::string>
    */
    void populateDescriptionCellArray() {
        if (m_bufferConfig.description_list.empty())
            return;
        std::vector<matioCpp::Variable> descrListVect;
        for (const auto& str : m_bufferConfig.description_list) {
            descrListVect.emplace_back(matioCpp::String("useless_name",str));
        }
        matioCpp::CellArray description_list("description_list", { m_bufferConfig.description_list.size(), 1 }, descrListVect);
        m_description_cell_array = description_list;
    }

    void resize(size_t new_size, std::shared_ptr<TreeNode<BufferInfo>> node) {

        // resize the variable
        auto variable = node->getValue();
        if (variable != nullptr) {
            variable->m_buffer.resize(new_size);
        }

        for (auto& [var_name, child] : node->getChildren()) {
            this->resize(new_size, child);
        }
    }

    void set_capacity(size_t new_size, std::shared_ptr<TreeNode<BufferInfo>> node) {

        // resize the variable
        auto variable = node->getValue();
        if (variable != nullptr) {
            variable->m_buffer.set_capacity(new_size);
        }

        for (auto& [var_name, child] : node->getChildren()) {
            this->set_capacity(new_size, child);
        }
    }


    BufferConfig m_bufferConfig;
    bool m_should_stop_thread{ false };
    std::mutex m_mutex_cv;
    std::condition_variable m_cv;
    std::shared_ptr<TreeNode<BufferInfo>> m_tree;

    std::function<double(void)> m_nowFunction{DefaultClock};
    std::thread m_save_thread;
    matioCpp::CellArray m_description_cell_array;
};

} // yarp::telemetry::experimental

#endif
