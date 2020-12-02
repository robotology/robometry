#include <vector>
#include <matioCpp/matioCpp.h>

using namespace std;

int main(int argc, char *argv[])
{
  matioCpp::File file = matioCpp::File::Create("test_timeseries.mat");


  // first we take care of the timestamps vector
  unsigned long int num_time_stamps = 3;
  vector<double> timestamps_vect{0.5, 1.0, 1.5};


  // convert from c++ vector to matioCpp vector
  matioCpp::Vector<double> timestamps("timestamps");
  timestamps = timestamps_vect;


  // now we initialize the proto-timeseries structure
  vector<matioCpp::Variable> timeSeries_data;


  // and the structures for the actual data too
  vector<matioCpp::Variable> data_left_arm;
  vector<matioCpp::Variable> data_right_arm;


  // here we create the matioCpp vector for the dimensions of the joints vector (#steps, #joints)
  vector<int> dimensions_left_vect{(int)num_time_stamps, 4};
  matioCpp::Vector<int> dimensions_left("dimensions");
  dimensions_left = dimensions_left_vect;


  // now we start populating the proto-structure for the left arm data
  // we start by creating an empty vector with the dimensions specified
  // (In practice, we will be creating the matioCpp vector as we did for the dimensions)
  data_left_arm.emplace_back(matioCpp::MultiDimensionalArray<double>("joints", {num_time_stamps,4}));

  // now we add also the dimensions vector to this proto-structure
  data_left_arm.emplace_back(dimensions_left);

  // and finally we convert the proto-structure into an actual matioCpp Struct
  matioCpp::Struct left_arm_struct("left_arm", data_left_arm);


  
  // now we repeat the steps above but for the right arm (in practice just changing the dimensions)
  vector<int> dimensions_right_vect{(int)num_time_stamps, 6};
  matioCpp::Vector<int> dimensions_right("dimensions");
  dimensions_right = dimensions_right_vect;

  data_right_arm.emplace_back(matioCpp::MultiDimensionalArray<double>("joints", {num_time_stamps,6}));
  data_right_arm.emplace_back(dimensions_right);
  matioCpp::Struct right_arm_struct("right_arm", data_right_arm);


  // now we create a vector with all the structs (signals) we have....
  vector<matioCpp::Struct> signalsVect{left_arm_struct, right_arm_struct};

  // ...and convert this vector into a matioCpp array of structures (StructArray)
  matioCpp::StructArray signals("signals", {1,2}, signalsVect);


  // Now, we populate the timeseries proto-struct with the timestamp vector (common for both signals) and the array of signals
  timeSeries_data.emplace_back(timestamps);
  timeSeries_data.emplace_back(signals);

  
  // finally we convert the proto-structure into a matioCpp Struct, ready for writing
  matioCpp::Struct timeSeries("output", timeSeries_data);
  

  file.write(timeSeries);

  return 0;
}

