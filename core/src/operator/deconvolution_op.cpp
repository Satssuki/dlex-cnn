////////////////////////////////////////////////////////////////
// > Copyright (c) 2017 by Contributors. 
// > https://github.com/cjmcv
// > brief  
// > author Jianming Chen
////////////////////////////////////////////////////////////////

#include "operator/deconvolution_op.h"
#include <sstream>

namespace dlex_cnn {
template <typename Dtype>
DeconvolutionOp<Dtype>::DeconvolutionOp() {
  op_type_ = "Deconvolution";
}

template <typename Dtype>
DeconvolutionOp<Dtype>::DeconvolutionOp(DeconvolutionOpParam param) {
  op_type_ = "Deconvolution";
  param_ = param;
}

template <typename Dtype>
DeconvolutionOp<Dtype>::~DeconvolutionOp() {}

template <typename Dtype>
int DeconvolutionOp<Dtype>::SetOpParam(const std::string &op_param_str) {
  std::string opt_str = op_param_str;
  param_.blas_enable = atoi(FetchSubStr(opt_str, "blas_enable:", ",").c_str());
  param_.kernel_channels = atoi(FetchSubStr(opt_str, "kernel_channels:", ",").c_str());
  param_.kernel_h = atoi(FetchSubStr(opt_str, "kernel_h:", ",").c_str());
  param_.kernel_w = atoi(FetchSubStr(opt_str, "kernel_w:", ",").c_str());
  param_.stride_h = atoi(FetchSubStr(opt_str, "stride_h:", ",").c_str());
  param_.stride_w = atoi(FetchSubStr(opt_str, "stride_w:", ",").c_str());
  param_.pad_w = atoi(FetchSubStr(opt_str, "pad_w:", ",").c_str());
  param_.dilation_h = atoi(FetchSubStr(opt_str, "dilation_h:", ",").c_str());
  param_.dilation_w = atoi(FetchSubStr(opt_str, "dilation_w:", ",").c_str());

  return 0;
}

template <typename Dtype>
std::string DeconvolutionOp<Dtype>::GenOpParamStr() const {
  std::stringstream param_str;
  param_str << "blas_enable:" << param_.blas_enable << ",kernel_channels:" << param_.kernel_channels
    << ",kernel_h:" << param_.kernel_h << ",kernel_w:" << param_.kernel_w
    << ",stride_h:" << param_.stride_h << ",stride_w:" << param_.stride_w
    << ",pad_h:" << param_.pad_h << ",pad_w:" << param_.pad_w
    << ",dilation_h:" << param_.dilation_h << ",dilation_w:" << param_.dilation_w << ",";
  return param_str.str();
}

template <typename Dtype>
int DeconvolutionOp<Dtype>::InferOutShape(std::vector<int> &in_shape, std::vector<int> &out_shape) {
  out_shape.clear();

  out_shape.push_back(in_shape[tind::eNum]);
  out_shape.push_back(param_.kernel_channels); // different from conv

  // just reverse the input and output dimension of conv.
  out_shape.push_back((in_shape[tind::eHeight] - 1) * param_.stride_h +
    (param_.dilation_h * (param_.kernel_h - 1) + 1) - 2 * param_.pad_h);
  out_shape.push_back((in_shape[tind::eWidth] - 1) * param_.stride_w +
    (param_.dilation_w * (param_.kernel_w - 1) + 1) - 2 * param_.pad_w);

  return 0;
}

template <typename Dtype>
int DeconvolutionOp<Dtype>::AllocBuf4Node(const std::vector<int> &in_shape,
  const std::vector<int> &out_shape,
  std::vector<std::shared_ptr<Tensor<Dtype>>> &data) const {
  data.clear();
  //printf("data and gradient: size() : %d, %d\n", data.size(), gradient_.size());
  data.push_back(std::make_shared<Tensor<Dtype>>(in_shape));

  // Weight (kernels for convolution). Different from conv, switch the input channels and output channels.
  data.push_back(std::make_shared<Tensor<Dtype>>(
    in_shape[tind::eChannels],
    param_.kernel_channels,
    param_.kernel_w,
    param_.kernel_h));

  // Blas. The same with conv, bias size is equal to output channel num
  if (param_.blas_enable)
    data.push_back(std::make_shared<Tensor<Dtype>>(out_shape[tind::eChannels], 1, 1, 1));

  return 0;
}

template <typename Dtype>
int DeconvolutionOp<Dtype>::AllocOpBuf4Train(const std::vector<int> &in_shape, const std::vector<int> &out_shape) {
  if (in_shape[tind::eNum] <= 0 || in_shape[tind::eChannels] <= 0 ||
    in_shape[tind::eHeight] <= 0 || in_shape[tind::eWidth] <= 0 ||
    in_shape[tind::eNum] > 5000 || in_shape[tind::eChannels] > 5000 ||
    in_shape[tind::eHeight] > 5000 || in_shape[tind::eWidth] > 5000) {
    DLOG_ERR("[ InnerProductOp::AllocOpBuf4Train ]: in_shape is invalid -> (%d, %d, %d, %d) \n",
      in_shape[tind::eNum], in_shape[tind::eChannels], in_shape[tind::eHeight], in_shape[tind::eWidth]);
    return -1;
  }

  diff_.clear();
  diff_.push_back(std::make_shared<Tensor<Dtype>>(in_shape));

  gradient_.clear();
  gradient_.push_back(std::make_shared<Tensor<Dtype>>(
    in_shape[tind::eChannels],
    param_.kernel_channels,
    param_.kernel_w,
    param_.kernel_h)); // Different from conv

  if (param_.blas_enable)
    gradient_.push_back(std::make_shared<Tensor<Dtype>>(out_shape[tind::eChannels], 1, 1, 1));

  return 0;
}

template <typename Dtype>
void DeconvolutionOp<Dtype>::Forward(
  const std::vector<std::shared_ptr<Tensor<Dtype>>> &prev,
  const std::vector<std::shared_ptr<Tensor<Dtype>>> &next) {
  const std::vector<int> prev_shape = prev[0]->get_shape();
  const std::vector<int> next_shape = next[0]->get_shape();

  const std::vector<int> prev_size = prev[0]->get_size();
  const std::vector<int> next_size = next[0]->get_size();

  const std::vector<int> kernel_shape = prev[1]->get_shape();
  const std::vector<int> kernel_size = prev[1]->get_size();

  const Dtype* prev_data = (Dtype *)prev[0]->GetPushCpuData();
  const Dtype* kernel_data = (Dtype *)prev[1]->GetPushCpuData();
  const Dtype* bias_data = (Dtype *)prev[2]->GetPushCpuData();
  Dtype* next_data = (Dtype *)next[0]->GetCpuData();

  // (1, channels*kernel_h*kernel_w, output_h*output_w)
  const int output_h = (prev_shape[tind::eHeight] - 1) * param_.stride_h +
    (param_.dilation_h * (param_.kernel_h - 1) + 1) - 2 * param_.pad_h;
  const int output_w = (prev_shape[tind::eWidth] - 1) * param_.stride_w +
    (param_.dilation_w * (param_.kernel_w - 1) + 1) - 2 * param_.pad_w;

  // The dimension of col_buffer is relevent to "kernel * prev". -> From the output of "kernel * prev" to col_buffer.
  int col_height = kernel_size[tind::e3D];// prev_shape[tind::eChannels] * param_.kernel_h * param_.kernel_w;
  int col_width = prev_size[tind::e2D];
  if (col_buffer_ == NULL)
    col_buffer_ = std::make_shared<Tensor<Dtype>>(1, 1, col_height, col_width);
  else if (col_buffer_->get_size()[tind::e4D] != 1 * 1 * col_height * col_width)
    col_buffer_.reset(new Tensor<Dtype>(1, 1, col_height, col_width));

  Dtype* col_data = (Dtype *)col_buffer_->GetPushCpuData();
  next[0]->SetCpuZero();
  for (int ni = 0; ni < prev_shape[tind::eNum]; ni++)
  {
    gemm_cpu(true, false, kernel_size[tind::e3D],
      prev_size[tind::e2D], prev_shape[tind::eChannels],
      (Dtype)1.0, kernel_data, prev_data + ni * prev_size[tind::e3D],
      (Dtype)0.0, col_data);

    //MatrixShow_float("kernel", (float *)kernel_data, 1, 1, kernel_shape[tind::eNum], kernel_size[tind::e3D]);
    //MatrixShow_float("prev", (float *)prev_data, 1, 1, prev_shape[tind::eChannels], prev_size[tind::e2D]);
    //MatrixShow_float("col", (float *)col_data, 1, 1, col_height, col_width);

    col2im_cpu(col_data, next_shape[tind::eChannels],
      next_shape[tind::eHeight], next_shape[tind::eWidth],
      param_.kernel_h, param_.kernel_w,
      param_.pad_h, param_.pad_w,
      param_.stride_h, param_.stride_w,
      param_.dilation_h, param_.dilation_w,
      next_data + ni * next_size[tind::e3D]);

    //MatrixShow_float("res", (float *)next_data, 1, next_shape[tind::eChannels], next_shape[tind::eHeight], next_shape[tind::eWidth]);
  }
  if (param_.blas_enable)
    add_bias(
    next_shape[tind::eNum],
    next_shape[tind::eChannels],
    next_size[tind::e2D],
    bias_data, next_data);
}

template <typename Dtype>
void DeconvolutionOp<Dtype>::Backward(
  const std::vector<std::shared_ptr<Tensor<Dtype>>> &prev,
  const std::vector<std::shared_ptr<Tensor<Dtype>>> &next,
  const std::vector<std::shared_ptr<Tensor<Dtype>>> &prev_diff,
  const std::vector<std::shared_ptr<Tensor<Dtype>>> &next_diff) {
  // data
  const std::vector<int> prev_shape = prev[0]->get_shape();
  const std::vector<int> next_shape = next[0]->get_shape();

  const std::vector<int> prev_size = prev[0]->get_size();
  const std::vector<int> next_size = next[0]->get_size();

  // diff
  const std::vector<int> prev_diff_shape = prev_diff[0]->get_shape();
  const std::vector<int> next_diff_shape = next_diff[0]->get_shape();

  const std::vector<int> prev_diff_size = prev_diff[0]->get_size();
  const std::vector<int> next_diff_size = next_diff[0]->get_size();

  // weight
  const std::vector<int> kernel_shape = prev[1]->get_shape();
  const std::vector<int> kernel_size = prev[1]->get_size();

  // bias
  //const std::vector<int> biasShape = prev[2]->get_shape();
  const std::vector<int> bias_size = prev[2]->get_size();

  const Dtype* prev_data = (Dtype*)prev[0]->GetPushCpuData();
  const Dtype* next_data = (Dtype*)next[0]->GetPushCpuData();
  Dtype* prev_diff_data = (Dtype*)prev_diff[0]->GetCpuData();
  Dtype* next_diff_data = (Dtype*)next_diff[0]->GetPushCpuData();
  Dtype *kernel_data = (Dtype*)prev[1]->GetPushCpuData();
  //Dtype *bias_data = (Dtype*)prev[2]->GetPushCpuData();

  const std::vector<int> col_shape = col_buffer_->get_shape();
  Dtype* col_data = (Dtype *)col_buffer_->GetPushCpuData();

  // update prev_diff
  prev_diff[0]->SetCpuZero();
  for (int ni = 0; ni < prev_shape[tind::eNum]; ni++) {
    //printf("address: %d\n", col_data);
    im2col_cpu<Dtype>(next_diff_data + ni*next_diff_size[tind::e3D], next_diff_shape[tind::eChannels],
      next_diff_shape[tind::eHeight], next_diff_shape[tind::eWidth],
      param_.kernel_h, param_.kernel_w,
      param_.pad_h, param_.pad_w,
      param_.stride_h, param_.stride_w,
      param_.dilation_h, param_.dilation_w,
      col_data);

    // col_shape[tind::eHeight] == kernel_size[tind::e3D]
    // kernel_num == prev channels
    gemm_cpu(false, false, prev_shape[tind::eChannels],
      col_shape[tind::eWidth], col_shape[tind::eHeight],
      (Dtype)1, kernel_data, col_data,
      (Dtype)0, prev_diff_data + ni * prev_diff_size[tind::e3D]);
  }

  // update weight Diff
  gradient_[0]->SetCpuZero();
  Dtype* kernel_gradient_data = (Dtype *)gradient_[0]->GetCpuData();
  for (int ni = 0; ni < prev_diff_shape[tind::eNum]; ni++) {
    im2col_cpu<Dtype>(next_diff_data + ni*next_diff_size[tind::e3D], next_diff_shape[tind::eChannels],
      next_diff_shape[tind::eHeight], next_diff_shape[tind::eWidth],
      param_.kernel_h, param_.kernel_w,
      param_.pad_h, param_.pad_w,
      param_.stride_h, param_.stride_w,
      param_.dilation_h, param_.dilation_w,
      col_data);

    gemm_cpu(false, true, prev_shape[tind::eChannels],
      kernel_size[tind::e3D], prev_size[tind::e2D],
      (Dtype)1.0, prev_data + ni * prev_size[tind::e3D], col_data,
      (Dtype)1.0, kernel_gradient_data);
  }
  div_inplace_cpu(kernel_size[tind::e4D], (Dtype)next_shape[tind::eNum], kernel_gradient_data);

  //update bias gradient
  gradient_[1]->SetCpuZero();
  Dtype* bias_gradient_data = (Dtype *)gradient_[1]->GetCpuData();

  backward_bias(
    next_diff_shape[tind::eNum],
    next_diff_shape[tind::eChannels],
    next_diff_size[tind::e2D],
    next_diff_data, bias_gradient_data);
  div_inplace_cpu(bias_size[tind::e4D], (Dtype)next_shape[tind::eNum], bias_gradient_data);
}

#ifdef USE_CUDA
template <typename Dtype>
void DeconvolutionOp<Dtype>::Forward_gpu(
  const std::vector<std::shared_ptr<Tensor<Dtype>>> &prev,
  const std::vector<std::shared_ptr<Tensor<Dtype>>> &next) {
  const std::vector<int> prev_shape = prev[0]->get_shape();
  const std::vector<int> next_shape = next[0]->get_shape();

  const std::vector<int> prev_size = prev[0]->get_size();
  const std::vector<int> next_size = next[0]->get_size();

  const std::vector<int> kernel_shape = prev[1]->get_shape();
  const std::vector<int> kernel_size = prev[1]->get_size();

  const Dtype* prev_data = (Dtype *)prev[0]->GetPushGpuData();
  const Dtype* kernel_data = (Dtype *)prev[1]->GetPushGpuData();
  const Dtype* bias_data = (Dtype *)prev[2]->GetPushGpuData();
  Dtype* next_data = (Dtype *)next[0]->GetGpuData();

  // (1, channels*kernel_h*kernel_w, output_h*output_w)
  const int output_h = (prev_shape[tind::eHeight] - 1) * param_.stride_h +
    (param_.dilation_h * (param_.kernel_h - 1) + 1) - 2 * param_.pad_h;
  const int output_w = (prev_shape[tind::eWidth] - 1) * param_.stride_w +
    (param_.dilation_w * (param_.kernel_w - 1) + 1) - 2 * param_.pad_w;

  // The dimension of col_buffer is relevent to "kernel * prev". -> From the output of "kernel * prev" to col_buffer.
  int col_height = kernel_size[tind::e3D];// prev_shape[tind::eChannels] * param_.kernel_h * param_.kernel_w;
  int col_width = prev_size[tind::e2D];
  if (col_buffer_ == NULL)
    col_buffer_ = std::make_shared<Tensor<Dtype>>(1, 1, col_height, col_width);
  else if (col_buffer_->get_size()[tind::e4D] != 1 * 1 * col_height * col_width)
    col_buffer_.reset(new Tensor<Dtype>(1, 1, col_height, col_width));

  Dtype* col_data = (Dtype *)col_buffer_->GetPushGpuData();
  next[0]->SetGpuZero();
  for (int ni = 0; ni < prev_shape[tind::eNum]; ni++)
  {
    gemm_gpu(CuHandleManager::cublas_handle(), true, false, kernel_size[tind::e3D],
      prev_size[tind::e2D], prev_shape[tind::eChannels],
      (Dtype)1.0, kernel_data, prev_data + ni * prev_size[tind::e3D],
      (Dtype)0.0, col_data);

    col2im_gpu(col_data, next_shape[tind::eChannels],
      next_shape[tind::eHeight], next_shape[tind::eWidth],
      param_.kernel_h, param_.kernel_w,
      param_.pad_h, param_.pad_w,
      param_.stride_h, param_.stride_w,
      param_.dilation_h, param_.dilation_w,
      next_data + ni * next_size[tind::e3D]);
  }
  if (param_.blas_enable)
    add_bias_gpu(next_shape[tind::eNum], next_shape[tind::eChannels], next_size[tind::e2D], bias_data, next_data);
}

template <typename Dtype>
void DeconvolutionOp<Dtype>::Backward_gpu(
  const std::vector<std::shared_ptr<Tensor<Dtype>>> &prev,
  const std::vector<std::shared_ptr<Tensor<Dtype>>> &next,
  const std::vector<std::shared_ptr<Tensor<Dtype>>> &prev_diff,
  const std::vector<std::shared_ptr<Tensor<Dtype>>> &next_diff) {
  // data
  const std::vector<int> prev_shape = prev[0]->get_shape();
  const std::vector<int> next_shape = next[0]->get_shape();

  const std::vector<int> prev_size = prev[0]->get_size();
  const std::vector<int> next_size = next[0]->get_size();

  // diff
  const std::vector<int> prev_diff_shape = prev_diff[0]->get_shape();
  const std::vector<int> next_diff_shape = next_diff[0]->get_shape();

  const std::vector<int> prev_diff_size = prev_diff[0]->get_size();
  const std::vector<int> next_diff_size = next_diff[0]->get_size();

  // weight
  const std::vector<int> kernel_shape = prev[1]->get_shape();
  const std::vector<int> kernel_size = prev[1]->get_size();

  // bias
  //const std::vector<int> biasShape = prev[2]->get_shape();
  const std::vector<int> bias_size = prev[2]->get_size();

  const Dtype* prev_data = (Dtype*)prev[0]->GetPushGpuData();
  const Dtype* next_data = (Dtype*)next[0]->GetPushGpuData();
  Dtype* prev_diff_data = (Dtype*)prev_diff[0]->GetGpuData();
  Dtype* next_diff_data = (Dtype*)next_diff[0]->GetPushGpuData();
  Dtype *kernel_data = (Dtype*)prev[1]->GetPushGpuData();
  //Dtype *bias_data = (Dtype*)prev[2]->GetPushGpuData();

  const std::vector<int> col_shape = col_buffer_->get_shape();
  Dtype* col_data = (Dtype *)col_buffer_->GetPushGpuData();

  // update prev_diff
  prev_diff[0]->SetGpuZero();
  for (int ni = 0; ni < prev_shape[tind::eNum]; ni++) {
    //printf("address: %d\n", col_data);
    im2col_gpu<Dtype>(next_diff_data + ni*next_diff_size[tind::e3D], next_diff_shape[tind::eChannels],
      next_diff_shape[tind::eHeight], next_diff_shape[tind::eWidth],
      param_.kernel_h, param_.kernel_w,
      param_.pad_h, param_.pad_w,
      param_.stride_h, param_.stride_w,
      param_.dilation_h, param_.dilation_w,
      col_data);

    // col_shape[tind::eHeight] == kernel_size[tind::e3D]
    // kernel_num == prev channels
    gemm_gpu(CuHandleManager::cublas_handle(), false, false, prev_shape[tind::eChannels],
      col_shape[tind::eWidth], col_shape[tind::eHeight],
      (Dtype)1, kernel_data, col_data,
      (Dtype)0, prev_diff_data + ni * prev_diff_size[tind::e3D]);
  }

  // update weight Diff
  gradient_[0]->SetGpuZero();
  Dtype* kernel_gradient_data = (Dtype *)gradient_[0]->GetGpuData();
  for (int ni = 0; ni < prev_diff_shape[tind::eNum]; ni++) {
    im2col_gpu<Dtype>(next_diff_data + ni*next_diff_size[tind::e3D], next_diff_shape[tind::eChannels],
      next_diff_shape[tind::eHeight], next_diff_shape[tind::eWidth],
      param_.kernel_h, param_.kernel_w,
      param_.pad_h, param_.pad_w,
      param_.stride_h, param_.stride_w,
      param_.dilation_h, param_.dilation_w,
      col_data);

    gemm_gpu(CuHandleManager::cublas_handle(), false, true, prev_shape[tind::eChannels],
      kernel_size[tind::e3D], prev_size[tind::e2D],
      (Dtype)1.0, prev_data + ni * prev_size[tind::e3D], col_data,
      (Dtype)1.0, kernel_gradient_data);
  }
  div_inplace_gpu(kernel_size[tind::e4D], (Dtype)next_shape[tind::eNum], kernel_gradient_data);

  //update bias gradient
  gradient_[1]->SetGpuZero();
  Dtype* bias_gradient_data = (Dtype *)gradient_[1]->GetGpuData();

  backward_bias_gpu(next_diff_shape[tind::eNum], next_diff_shape[tind::eChannels], next_diff_size[tind::e2D], next_diff_data, bias_gradient_data);
  div_inplace_gpu(bias_size[tind::e4D], (Dtype)next_shape[tind::eNum], bias_gradient_data);
}
#endif

INSTANTIATE_CLASS(DeconvolutionOp);

}//namespace