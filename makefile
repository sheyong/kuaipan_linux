#makefile

objects = kp_oauth.o  kp_api.o

oauth_path = ./kuaipan_sdk/kp_oauth.cpp
api_path = ./kuaipan_sdk/kp_api.cpp
sycn_path = kuaipan_demo/kp_sycn.cpp
libs = -ljson -loauth -lsqlite3 -lcurl
 
all: kp_sycn sdk_unittest.o

kp_sycn:  $(objects)
	g++ -o kp_sycn $(sycn_path) $(objects) $(libs)

kp_oauth.o: $(oauth_path)
	g++ -c $(oauth_path)

kp_api.o: $(api_path)
	g++ -c $(api_path)

sdk_unittest.o: sdk_unittest/unittest.cpp $(objects)
	g++ -o sdk_unittest.o sdk_unittest/unittest.cpp $(objects) $(libs)

clean:
	 rm  $(objects) kp_sycn sdk_unittest.o
