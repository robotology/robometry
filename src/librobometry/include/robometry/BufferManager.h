/*
 * Copyright (C) 2006-2021 Istituto Italiano di Tecnologia (IIT)
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the
 * BSD-3-Clause license. See the accompanying LICENSE file for details.
 */

#ifndef ROBOMETRY_BUFFER_MANAGER_H
#define ROBOMETRY_BUFFER_MANAGER_H

#include <initializer_list>
#include <robometry/Buffer.h>
#include <robometry/BufferConfig.h>
#include <robometry/TreeNode.h>

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

#ifndef ROBOMETRY_UNUSED
#  define ROBOMETRY_UNUSED(x) (void)x;
#endif // ROBOMETRY_UNUSED

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


namespace robometry {

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

// matiomatioCppCanConcatenate<T>::value is true when T has the T::value_type member. If this is true, then we check
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
* @brief Class that aggregates the robometry::Buffer and some other
* info(e.g. dimensions) used by the robometry::BufferManager
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
    units_of_measure_t m_units_of_measure;

    BufferInfo() = default;
    BufferInfo(const BufferInfo& other) = default;

    BufferInfo(BufferInfo&& other) = default;

    // This method fills the m_convert_to_matioCpp lambda with a function able to convert the Buffer
    // into a matioCpp variable. This method is called when pushing the first time to a channel,
    // exploiting the fact that the push_back method is a template method
    // (and thus it is clear the type of input, necessary when casting std::any).
    template<typename T>
    void createMatioCppConvertFunction()
    {
        // First check if we can use the matioCpp::make_variable function with the input type T.
        // If not, the input type is not compatible with matioCpp
        static_assert(matioCpp::is_make_variable_callable<T>::value, "The selected type cannot be used with matioCpp.");

        // The lambda is generated only the first time that we push to a channel.
        // We are enforcing that the type pushed with push_back is always the same.
        if (m_convert_to_matioCpp)
        {
            return;
        }

        // The matioCpp::make_variable_output metafunction provides the type that would be output by matioCpp::make_variable
        using matioCppType = typename matioCpp::make_variable_output<T>::type;

        // Start filling the m_convert_to_matioCpp lambda. The lambda will take as input the desired name and will output a matioCpp::Variable.
        m_convert_to_matioCpp = [this](const std::string& name)
        {
            size_t num_instants = this->m_buffer.size();

            //---
            //SCALAR CASE
            //if the input data is numeric (scalar, vector, multi-dimensional array), then we concatenate on the last dimension
            if constexpr (matioCppCanConcatenate<matioCppType>::value)
            {
                //the scalar types in matioCpp have the member T::value_type, that is the type of each single element.
                using elementType = typename matioCppType::value_type;

                dimensions_t fullDimensions = this->m_dimensions;
                fullDimensions.push_back(num_instants);

                // The output is a multi dimensional array of dimensions n+1, where the last dimension is the number of time instants.
                matioCpp::MultiDimensionalArray<elementType> outputVariable(name, fullDimensions);

                size_t t = 0;
                for (auto& _cell : this->m_buffer) {
                    //We convert std::any type using the input type T.
                    const T& cellCasted = std::any_cast<T>(_cell.m_datum);

                    matioCpp::Span<const elementType> matioCppSpan;

                    matioCppType matioCppVariable;

                    //We convert the cell to a matioCpp variable only if we are not able to create a Span (for example in case of scalars).
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

                    size_t startIndex = this->m_dimensions_factorial * t; //We concatenate on the last dimension. Suppose that the channel stores matrices of size 3x2.
                                                                          //The output variable is a 3x2xn matrix, where n is the number of elements in the buffer.
                                                                          //If we consider the output buffer as a linear vector, the element at time t would start
                                                                          //from location 6*t and end at 6*(t+1)

                    //matioCppSpan.size() should be equal to m_dimensions_factorial, but we avoid to perform this check for each input.
                    //Hence, with std::min we make sure to avoid reading or wrinting in wrong pieces of memory
                    for (size_t i = 0; i < std::min(static_cast<size_t>(matioCppSpan.size()), this->m_dimensions_factorial); ++i)
                    {
                        outputVariable[startIndex + i] = matioCppSpan[i]; //we copy the new element in the corresponding position inside the variable
                    }
                    ++t;
                }
                return outputVariable;
            }

            //---
            //STRUCT CASE
            else if constexpr(std::is_same_v<matioCppType, matioCpp::Struct>) //if the input is a struct, we use a struct array
            {
                //The output variable would be a struct array of dimensions t.
                matioCpp::StructArray outputVariable(name, {1,num_instants});

                size_t i = 0;
                for (auto& _cell : this->m_buffer) {
                    matioCpp::Struct element = matioCpp::make_variable("element", std::any_cast<T>(_cell.m_datum));

                    if (i == 0)
                    {
                        outputVariable.addFields(element.fields()); //Just for the first element, we specify the set of fields in the array
                    }

                    outputVariable.setElement(i, element);
                    ++i;
                }
                return outputVariable;
            }

            //---
            //CELL CASE (default)
            else //otherwise we use a cell array
            {
                //The output variable would be a cell array of dimensions t.
                matioCpp::CellArray outputVariable(name, {1,num_instants});

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
 * @brief The SaveCallback may need to know if it is called in a periodic fashion or is the
 * last call before deallocating the class
 *
 */
enum class SaveCallbackSaveMethod {
    periodic, last_call
};

/**
 * @brief Class that manages the buffers associated to the channels of the telemetry.
 * Each BufferManager can handle different types of data, the number of samples is defined in the configuration and
 * it is the same for every channel.
 * On the other hand the data inside the channels can have different dimensionality(e.g. 1x1, 2x3 etc).
 * It contains utilities for saving the data of the channels in mat files, and to save/read the configuration
 * to/from a json file.
 *
 */
class BufferManager {

public:
    /**
     * @brief Construct an empty BufferManager object.
     * For being used it has to be configured afterwards.
     *
     */
    BufferManager();

    /**
     * @brief Construct a new BufferManager object, configuring it via
     * the robometry::BufferConfig.
     *
     * @param[in] _bufferConfig The struct containing the configuration for the BufferManager.
     */
    BufferManager(const BufferConfig& _bufferConfig);

    /**
     * @brief Destroy the BufferManager object.
     * If auto_save is enabled, it saves to file the remaining data in the buffer.
     *
     */
    ~BufferManager();

    /**
     * @brief Enable the save thread with _save_period seconds of period.
     * If the thread has been started yet in the configuration through
     * BufferConfing, it skips it.
     *
     * @param[in] _save_period The period in seconds of the save thread.
     * @return true on success, false otherwise.
     */
    bool enablePeriodicSave(double _save_period);

    /**
     * @brief Configure the BufferManager through a BufferConfig object.
     *
     * @param[in] _bufferConfig The struct containing the configuration parameters.
     * @return true on success, false otherwise.
     */
    bool configure(const BufferConfig& _bufferConfig);

    /**
     * @brief Get the BufferConfig object representing the actual configuration.
     *
     * @return The BufferConfig object.
     */
    BufferConfig getBufferConfig() const;
    /**
     * @brief Set the file name that will be created by the BufferManager.
     *
     * @param[in] filename The file name to be set.
     */
    void setFileName(const std::string& filename);

    /**
     * @brief Set the path where the files will be saved.
     *
     * @param[in] path The path to be set.
     */
    void setDefaultPath(const std::string& path);

    /**
    * @brief Enable the zlib compression.
    *
    * @param[in] flag for enabling/disabling compression.
    */
    void enableCompression(bool enable_compression);

    /**
     * @brief Set the description list that will be saved in all the files.
     *
     * @param[in] description The description to be set.
     */
    void setDescriptionList(const std::vector<std::string>& description_list);

    /**
     * @brief Resize the Buffer/s.
     *
     * @param[in] new_size The new size to be resized to.
     */
    void resize(size_t new_size);

    /**
     * @brief Set the capacity of Buffer/s.
     *
     * @param[in] new_size The new size.
     */
    void set_capacity(size_t new_size);

    /**
     * @brief Add a channel(variable) to the BufferManager.
     * The channels have to be unique in the BufferManager.
     *
     * @param[in] channel Pair representing the channel to be added.
     * @return true on success, false otherwise.
     */
    bool addChannel(const ChannelInfo& channel);

    /**
     * @brief Add a list of channels(variables) to the BufferManager.
     * The channels have to be unique in the BufferManager.
     *
     * @param[in] channels List of pair representing the channels to be added.
     * @return true on success, false otherwise.
     */
    bool addChannels(const std::vector<ChannelInfo>& channels);

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
        bufferInfo->template createMatioCppConvertFunction<T>();

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
     * have a number of samples greater than the robometry::BufferConfig::data_threshold.
     * If robometry::BufferConfig::data_threshold is greater than robometry::BufferConfig::n_samples
     * this check is skipped.
     *
     * @param[in] flush_all Flag for forcing the save of whatever is contained in the channels.
     * @return true on success, false otherwise.
     */
    bool saveToFile(bool flush_all=true);

    /**
     * @brief Save the content of all the channels into a file.
     * If flush_all is set to false, it saves only the content of the channels that
     * have a number of samples greater than the robometry::BufferConfig::data_threshold.
     * If robometry::BufferConfig::data_threshold is greater than robometry::BufferConfig::n_samples
     * this check is skipped.
     *
     * @param[in] flush_all Flag for forcing the save of whatever is contained in the channels.
     * @param[out] file_name_path path name of the matfile without the suffix .mat
     * @return true on success, false otherwise.
     */
    bool saveToFile(std::string& file_name_path, bool flush_all=true);

    /**
     * @brief Set the now function, by default is std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count().
     * @param[in] now The now function
     * @return true on success, false otherwise.
     */
    bool setNowFunction(std::function<double(void)> now);


    /**
     * @brief Set the saveCallback function. Thanks to this function you can save additional data
     * type along with the matfile salve by telemetry
     * @param[in] saveCallback The saveCallback function
     * @return true on success, false otherwise.
     */
    bool setSaveCallback(std::function<bool(const std::string&, const SaveCallbackSaveMethod& method)> saveCallback);


private:
    static double DefaultClock();

    void periodicSave();

    matioCpp::Struct createTreeStruct(const std::string& node_name,
                                      std::shared_ptr<TreeNode<BufferInfo>> tree_node,
                                      bool flush_all);

    matioCpp::Struct createElementStruct(const std::string& var_name,
                                         std::shared_ptr<BufferInfo> buffInfo,
                                         bool flush_all) const;

    /**
    * This is an helper function that can be used to generate the file indexing accordingly to the
    * content of `m_bufferConfig.file_indexing`
    * @return a string containing the index
    */
    std::string fileIndex() const;

    /**
    * This is an helper function that will be disappear the day matio-cpp
    * will support the std::vector<std::string>
    */
    void populateDescriptionCellArray();

    void resize(size_t new_size, std::shared_ptr<TreeNode<BufferInfo>> node);

    void set_capacity(size_t new_size, std::shared_ptr<TreeNode<BufferInfo>> node);


    BufferConfig m_bufferConfig;
    bool m_should_stop_thread{ false };
    std::mutex m_mutex_cv;
    std::condition_variable m_cv;
    std::shared_ptr<TreeNode<BufferInfo>> m_tree;

    std::function<double(void)> m_nowFunction{DefaultClock};
    std::function<bool(const std::string&, const SaveCallbackSaveMethod& method)> m_saveCallback{};

    std::thread m_save_thread;
    matioCpp::CellArray m_description_cell_array;
};

} // robometry

#endif
