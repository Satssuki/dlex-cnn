////////////////////////////////////////////////////////////////
// > Copyright (c) 2017 by Contributors. 
// > https://github.com/cjmcv
// > brief  
// > author Jianming Chen
////////////////////////////////////////////////////////////////

#ifndef DLEX_COMMON_HPP_
#define DLEX_COMMON_HPP_

#include <iostream>

namespace dlex_cnn
{
#ifndef FLT_MIN
#define FLT_MIN 1.175494351e-38F 
#endif

// class instantiate
#define INSTANTIATE_CLASS(classname) \
  char gInstantiationGuard##classname; \
  template class classname<float>; \
  template class classname<double>

#define INSTANTIATE_CLASS_NOR(classname) \
  char gInstantiationGuard##classname; \
  template class classname<int>; \
  template class classname<float>; \
  template class classname<double>

// log control
#define DLOG_ERR(format, ...) fprintf(stderr,"ERROR: "#format"\n", ##__VA_ARGS__);
#define DLOG_INFO(format, ...) fprintf(stdout,"INFO: "#format"\n", ##__VA_ARGS__);
#define DLOG_WARN(format, ...) fprintf(stdout,"WARN: "#format"\n", ##__VA_ARGS__);

// val check
#define DCHECK(val) (((val)==0)? false:true)

#define DCHECK_EQ(val1, val2) (((val1)==(val2))? true:false)
#define DCHECK_NE(val1, val2) (((val1)!=(val2))? true:false)

#define DCHECK_LE(val1, val2) (((val1)<=(val2))? true:false)
#define DCHECK_LT(val1, val2) (((val1)< (val2))? true:false)

#define DCHECK_GE(val1, val2) (((val1)>=(val2))? true:false)
#define DCHECK_GT(val1, val2) (((val1)> (val2))? true:false)

// assert
//#define CUDNN_GCK(condition) \
//  do { \
//    cudnnStatus_t status = condition; \
//	if(!GCK_EQ(status, CUDNN_STATUS_SUCCESS)) { \
//		DLOG_ERR(cudnnGetErrorString(status)); \
//		throw(1);	\
//		} \
//    } while (0)
//
// string process
std::string fetchSubStr(std::string &src_str, std::string start_str, std::string end_str);

}

#endif //DLEX_COMMON_HPP_