////////////////////////////////////////////////////////////////
// > Copyright (c) 2017 by Contributors. 
// > https://github.com/cjmcv
// > brief  Simple activation functions.
// > author Jianming Chen
////////////////////////////////////////////////////////////////

#ifndef DLEX_OP_ACTIVATION_HPP_
#define DLEX_OP_ACTIVATION_HPP_

#include "operator_base.h"
#include "tensor.h"
#include "util/math_functions.h"
#include <functional> 
#include <algorithm>

namespace dlex_cnn {
namespace tind {
  enum Activation { eReLU, eSigmoid, eTanh };
}

struct ActivationOpParam : public OpParam {
  tind::Activation activation_type = tind::eReLU;

  // relu
  float negative_slope = 0;
};

template <typename Dtype>
class ActivationOp : public Op<Dtype> {
public:
  ActivationOp();
  ActivationOp(ActivationOpParam param);
  virtual ~ActivationOp();
  inline virtual int SetOpParam(ActivationOpParam op_param) { param_ = op_param; SetOpFunc(); return 0; };
  virtual int SetOpParam(const std::string &op_param_str) override;

private:
  // For Forward.
  std::function<Dtype(Dtype)> p_act;
  // For Backward.
  std::function<Dtype(Dtype, Dtype)> p_rev_act;

  inline Dtype relu(Dtype x) { return std::max(x, Dtype(0)) + param_.negative_slope * std::min(x, Dtype(0)); }
  inline Dtype rev_relu(Dtype px, Dtype diff_next) { return diff_next * ((px > 0) + param_.negative_slope * (px <= 0)); }

  inline Dtype sigmoid(Dtype x) { return 1. / (1. + exp(-x)); }
  inline Dtype rev_sigmoid(Dtype nx, Dtype diff_next) { return diff_next * nx * (1. - nx); }

  inline Dtype tanh(Dtype x) { return std::tanh(x); }
  inline Dtype rev_tanh(Dtype nx, Dtype diff_next) { return diff_next * (1 - nx * nx); }

private:
  // Bind activation function.
  int SetOpFunc();

  inline virtual const std::string &get_op_type() const override { return op_type_; };
  inline virtual const int get_op_category() const override { return tind::eNormOp; };
  inline virtual std::vector<std::shared_ptr<Tensor<Dtype>>> &get_op_diff() override { return diff_; };

  virtual std::string GenOpParamStr() const override;
  virtual int InferOutShape(std::vector<int> &in_shape, std::vector<int> &out_shape) override;
  virtual int AllocBuf4Node(const std::vector<int> &in_shape,
    const std::vector<int> &out_shape,
    std::vector<std::shared_ptr<Tensor<Dtype>>> &data) const override;
  virtual int AllocOpBuf4Train(const std::vector<int> &in_shape, const std::vector<int> &out_shape) override;

  virtual void Forward(
    const std::vector<std::shared_ptr<Tensor<Dtype>>> &prev,
    const std::vector<std::shared_ptr<Tensor<Dtype>>> &next) override;
  virtual void Backward(
    const std::vector<std::shared_ptr<Tensor<Dtype>>> &prev,
    const std::vector<std::shared_ptr<Tensor<Dtype>>> &next,
    const std::vector<std::shared_ptr<Tensor<Dtype>>> &prev_diff,
    const std::vector<std::shared_ptr<Tensor<Dtype>>> &next_diff) override;
#ifdef USE_CUDA
  virtual void Forward_gpu(
    const std::vector<std::shared_ptr<Tensor<Dtype>>> &prev,
    const std::vector<std::shared_ptr<Tensor<Dtype>>> &next) override;
  virtual void Backward_gpu(
    const std::vector<std::shared_ptr<Tensor<Dtype>>> &prev,
    const std::vector<std::shared_ptr<Tensor<Dtype>>> &next,
    const std::vector<std::shared_ptr<Tensor<Dtype>>> &prev_diff,
    const std::vector<std::shared_ptr<Tensor<Dtype>>> &next_diff) override;
#endif

private:
  std::string op_type_;
  ActivationOpParam param_;
  std::vector<std::shared_ptr<Tensor<Dtype>>> diff_;
};


/////////////////////////////  Creator   /////////////////////////////////
template <typename Dtype>
ActivationOp<Dtype> *CreateOp(ActivationOpParam &param) {
  ActivationOp<Dtype> *op = NULL;
  op = new ActivationOp<Dtype>(param);
  return op;
}
template <typename Dtype>
std::shared_ptr<Op<Dtype>> CreateActivationOp() {
  return std::shared_ptr<Op<Dtype>>(new ActivationOp<Dtype>());
}
}
#endif
