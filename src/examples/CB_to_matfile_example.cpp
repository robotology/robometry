/*
 * Copyright (C) 2006-2020 Istituto Italiano di Tecnologia (IIT)
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the
 * BSD-3-Clause license. See the accompanying LICENSE file for details.
 */

#include <boost/circular_buffer.hpp>
#include <iostream>
#include <chrono>
#include <thread>
#include <random>
#include <matioCpp/matioCpp.h>
#include <mutex>

#include <yarp/telemetry/experimental/Buffer.h>
#include <yarp/os/Time.h>
#include <yarp/os/Network.h>

#include <memory>

using namespace std;
using namespace yarp::telemetry::experimental;
using namespace yarp::os;

std::mutex lock_mut;

class storeData {

  private:
    bool closing;
    double period;
    double wait_interval;
    vector<Record> local_collection; // stores on the read-thread the values from the buffer
    std::shared_ptr<boost::circular_buffer<Record>>  cb; // shared pointer to circular buffer
  public:

    // constructor of the read/save class. Initialized with the shared pointer and the read period
    storeData(std::shared_ptr<boost::circular_buffer<Record>> _cb, const double& _period) : cb(_cb), period(_period)
    {
      closing = false;
    }

    // destructor
    virtual ~storeData(){};

    // function that periodically reads the buffer
    bool readBuffer()
    {
      while(!closing)
      {

        // we use yarp os Time to check how long it takes for next loop
        auto start = yarp::os::Time::now();

        // we need to check if the buffer has actual data
        if(cb->empty())
        {
          cout << "the buffer is empty! check the data receiver is still ok" << endl;
          wait_interval = period - (yarp::os::Time::now() - start);
          cout << "Waiting for " << wait_interval << " seconds" << endl;
          if (wait_interval > 0) yarp::os::Time::delay(wait_interval);
          continue;
        }

        // here we read and remove all elements. Each element we retrieve from the circular buffer should be removed (pop_back) to prevent reread
        lock_mut.lock();
        for (long unsigned int i=0; i < cb->size(); i++)
        {
          // print the elements stored in the vector (for confirmation)
          auto castedDatum = any_cast<vector<int>>(cb->back().m_datum);
          for (auto f = castedDatum.begin(); f != castedDatum.end(); ++f)
            cout << *f << ' ';
          cout << endl;
          // store the elements into a local collection (independent of the circular buffer to allow reading more elements)
          local_collection.push_back(cb->back());
          cout << "populated local collection" << endl;

          // clear the read entry
          cb->pop_back();
        }
        lock_mut.unlock();

        wait_interval = period - (yarp::os::Time::now() - start);
        cout << "Waiting for " << wait_interval << " seconds" << endl;
        if (wait_interval > 0) yarp::os::Time::delay(wait_interval);
      }
      cout << "stopping reading from buffer" << endl;
      return true;
    }

    // function that converts the data on Record to matioCpp elements, and saves everything to a mat file
    bool saveToFile()
    {

      // create copy of the local collection (this acts as a second buffer)
      lock_mut.lock();
      vector<Record> _collection_copy = local_collection;
      lock_mut.unlock();
      cout << "local copy created " << endl;

      vector<int> linear_matrix;
      vector<double> timestamp_vector;

      // the number of timesteps is the size of our collection
      auto num_timesteps = _collection_copy.size();
      cout << "num timesteps: " << num_timesteps << endl;
      if(num_timesteps < 1)
      {
        cout << "not enough data points collected " << endl;
        return false;
      }
      // we assume the size of the data is the same for every timestep (is there any situation this would not be the case?)
      int size_datum = any_cast<vector<int>>(_collection_copy[0].m_datum).size();
      cout << "size of datum: " << size_datum << endl;

      // we first collapse the matrix of data into a single vector, in preparation for matioCpp convertion
      cout << "linearizing matrix..." << endl;
      for (auto& _cell : _collection_copy)
      {
        for (auto& _el : any_cast<vector<int>>(_cell.m_datum))
        {
          linear_matrix.push_back(_el);
        }
        timestamp_vector.push_back(_cell.m_ts);
      }
      cout << "matrix linearized " << endl;

      // now we start the matioCpp convertion process

      // first create timestamps vector
      matioCpp::Vector<double> timestamps("timestamps");
      timestamps = timestamp_vector;

      // and the structures for the actual data too
      vector<matioCpp::Variable> test_data;

      // now we create the vector for the dimensions
      vector<int> dimensions_data_vect{(int)num_timesteps, size_datum};
      matioCpp::Vector<int> dimensions_data("dimensions");
      dimensions_data = dimensions_data_vect;

      // now we populate the matioCpp matrix
      matioCpp::MultiDimensionalArray<int> out("test", {(unsigned long int)size_datum, (unsigned long int)num_timesteps}, linear_matrix.data());
      test_data.emplace_back(out); // Data

      test_data.emplace_back(dimensions_data); // dimensions vector

      test_data.emplace_back(matioCpp::String("name", "test data")); // name of the signal

      // we store it as a matioCpp struct
      matioCpp::Struct data_struct("test_data", test_data);

      // now we create the vector that stores different signals (in case we had more than one)
      vector<matioCpp::Variable> signalsVect;
      signalsVect.emplace_back(data_struct);

      // and the matioCpp struct for these signals
      matioCpp::Struct signals("signals", signalsVect);

      // now we initialize the proto-timeseries structure
      vector<matioCpp::Variable> timeSeries_data;

      // the timestamps vector is stored in parallel to the signals
      timeSeries_data.emplace_back(timestamps);
      timeSeries_data.emplace_back(signals);

      matioCpp::Struct timeSeries("Output", timeSeries_data);

      // and finally we write the file
      matioCpp::File file = matioCpp::File::Create("test_cb.mat");
      file.write(timeSeries);

      return true;
    }

    bool close()
    {
      return closing = true;
    }
};


int main()
{
  yarp::os::Network yarp;

  /* generate random integer vector with 10 entries */
  random_device rnd_device;
  mt19937 mersenne_engine {rnd_device()};  // Generates random integers
  uniform_int_distribution<int> dist {1, 52};
  auto gen = [&dist, &mersenne_engine](){
                 return dist(mersenne_engine);
             };


  string input_comm = "";
  vector<int> vec(10);
  /**************************************************/

  // Initialization of our Buffer (3 entries, type vector<int>)
  Buffer cb(3);
  double period = 5; // period for the reading of the buffer

  // Initialization of our reading and saving to file class - uses the shared-pointer for reading the circular buffer
  storeData storer(cb.getBufferSharedPtr(), period);

  // only the reading function will be ran in a separate thread
  thread my_thread(&storeData::readBuffer, std::ref(storer));

  // loop that populates the circular buffer. write "no" when prompted in order to save the stored data to a mat file
  while(true)
  {
    generate(begin(vec), end(vec), gen); // generates the random vector

    // we lock before we populate the circular buffer to prevent conflicts with reading
    lock_mut.lock();
    cb.push_back(Record({yarp::os::Time::now(), vec}));
    lock_mut.unlock();

    // user input -> say "no" to close the loop and generate the mat file
    cout << "shall we continue?" << endl;
    cin >> input_comm;
    cout << input_comm << endl;
    if(input_comm.find("no") != std::string::npos)
      break;
  }

  storer.close(); // close the reading loop
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  cout << "joining thread..." << endl;
  my_thread.join(); // terminate thread
  cout << "saving to file" << endl;
  storer.saveToFile(); // save to mat file
  cout << "closing" << endl;
  return 0;
}

