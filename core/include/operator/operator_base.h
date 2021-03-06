////////////////////////////////////////////////////////////////
// > Copyright (c) 2017 by Contributors. 
// > https://github.com/cjmcv
// > brief  
// > author Jianming Chen
////////////////////////////////////////////////////////////////

#ifndef DLEX_OP_BASE_HPP_
#define DLEX_OP_BASE_HPP_

//#include <iostream>
#include <vector>
#include <memory>
#include <stdlib.h>

//#include "dlex_datatype.h"
#include "tensor.h"

namespace dlex_cnn{
namespace tind {
  enum OpCategory { eNormOp, eLossOp, eHybridOp };
}

struct OpParam {};

template <typename Dtype>
class Op {
public:
  explicit Op();
  virtual ~Op();

  // Set operator's parameters by specific string from model file.
  virtual int SetOpParam(const std::string &op_param_str) { return -1; };

  // Generate operator's parameter string for model saving.
  virtual std::string GenOpParamStr() const { return ""; };

  virtual int InferOutShape(std::vector<int> &in_shape, std::vector<int> &out_shape) { return -1; };

  // Allocate memory buffer for node, including data/weight/blas.
  virtual int AllocBuf4Node(const std::vector<int> &in_shape,
    const std::vector<int> &out_shape,
    std::vector<std::shared_ptr<Tensor<Dtype>>> &data) const {
    return -1;
  };

  // Allocate memory buffer for training, including gradient and difference in operator
  virtual int AllocOpBuf4Train(const std::vector<int> &in_shape, const std::vector<int> &out_shape) { return -1; };

  virtual std::vector<std::shared_ptr<Tensor<Dtype>>> &get_op_gradient() { return gradient_; };
  virtual std::vector<std::shared_ptr<Tensor<Dtype>>> &get_op_diff() { return diff_; };
  virtual const std::string &get_op_type() const { return " "; };
  virtual const int get_op_category() const { return tind::eNormOp; };

  virtual void Forward(
    const std::vector<std::shared_ptr<Tensor<Dtype>>> &prev,
    const std::vector<std::shared_ptr<Tensor<Dtype>>> &next) {};
  virtual void Backward(
    const std::vector<std::shared_ptr<Tensor<Dtype>>> &prev,
    const std::vector<std::shared_ptr<Tensor<Dtype>>> &next,
    const std::vector<std::shared_ptr<Tensor<Dtype>>> &prev_diff,
    const std::vector<std::shared_ptr<Tensor<Dtype>>> &next_diff) {};
#ifdef USE_CUDA
  virtual void Forward_gpu(
    const std::vector<std::shared_ptr<Tensor<Dtype>>> &prev,
    const std::vector<std::shared_ptr<Tensor<Dtype>>> &next) {};
  virtual void Backward_gpu(
    const std::vector<std::shared_ptr<Tensor<Dtype>>> &prev,
    const std::vector<std::shared_ptr<Tensor<Dtype>>> &next,
    const std::vector<std::shared_ptr<Tensor<Dtype>>> &prev_diff,
    const std::vector<std::shared_ptr<Tensor<Dtype>>> &next_diff) {};
#endif

private:
  // Gradient, including weightGradient/blasGradient, has the same size with weight.
  // Will be used in optimizer for updating weight/blas during Backward operation.
  // Should be cleaned at each Backward iteration
  std::vector<std::shared_ptr<Tensor<Dtype>>> gradient_;

  // Difference, has the same size with input data, will be used in Backward operation for updating gradient;
  std::vector<std::shared_ptr<Tensor<Dtype>>> diff_;
  OpParam param_;
};

}

#endif //DLEX_OP_BASE_HPP_