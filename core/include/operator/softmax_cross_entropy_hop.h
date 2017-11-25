////////////////////////////////////////////////////////////////
// > Copyright (c) 2017 by Contributors. 
// > https://github.com/cjmcv
// > brief  
// > author Jianming Chen
////////////////////////////////////////////////////////////////

#ifndef DLEX_OP_SOFTMAX_CROSS_HPP_
#define DLEX_OP_SOFTMAX_CROSS_HPP_

#include "configure.h"
#include "operator_base.h"
#include "operator_hybrid.h"
#include "operator/cross_entropy_lop.h"
#include "operator/softmax_op.h"
#include "tensor.h"

namespace dlex_cnn
{
	struct SoftmaxCrossEntropyLossHOpParam : public HybridOpParam
	{

	};

	//////////////////////////////////////////////////////////////////////////
	// cross entropy
	// -- ��������ṹʱ��Ϊÿ��node��������������ֱ��������loss��node��ÿ��loss node�ĺ��涼���Զ�һ��outputNode��Ϊ���档
	// outputNode��Op����ǰ���򴫲���ֻ��data��diff��data[0]Ϊoutput/[1]Ϊlabel/[2]Ϊloss��diff����ռ����ڷ���
	// ����һ��node����softmaxLoss����softmaxLoss����softmaxOp����softmaxLoss��softmaxOp��
	// ǰ��node��cpu_dataΪ���ݣ���Ϊprev�����Ϊsoftmax��output��loss����һ��node��operator_output node����
	// ������diff��prev��next(output+label����outNode��data��)��prevdiff��nextdiff(outNode��diff)
	// ǰ����Ĳ����Ӿ���prev

	// ǰ���ܣ���������һ�����ݣ����output[0],label[1]��loss[2]
	// ǰ��softmax������һ�����ݣ����output��loss����output���������[1]��ȡlabel,���loss��ת���������

	// �����ܣ���prev��next(output+label����outNode��data��)��prevdiff��nextdiff(outNode��diff��ֻ����ȡ)
	// ����getDiff�� prev��next(output+label����outNode��data��)��prevdiff(��nextdiff)��nextdiff(outNode��diff��ֻ����ȡ)�� ���õ���diff��prevdiff
	// ����softmax�� prev, lastOutput_��outNode��data����ȡ0��, prevdiff, lastDiff_��getDiff�Ľ����

	template <typename Dtype>
	class SoftmaxCrossEntropyLossHOp : public HybridOp<Dtype>
	{
	public:
		SoftmaxCrossEntropyLossHOp();
		SoftmaxCrossEntropyLossHOp(SoftmaxCrossEntropyLossHOpParam param);
		virtual ~SoftmaxCrossEntropyLossHOp();
		inline virtual int setOpParam(SoftmaxCrossEntropyLossHOpParam op_param) { param_ = op_param; return 0; };
		virtual int setOpParam(const std::string &op_param_str) override;

	private:
		inline virtual const std::string &getOpType() const override { return op_type_; };
		inline virtual const int getOpCategory() const override { return tind::eHybridOp; };
		inline virtual std::vector<std::shared_ptr<Tensor<Dtype>>> &getOpDiff() override { return diff_; };

		virtual std::string genOpParamStr() const override;
		virtual int inferOutShape(std::vector<int> &in_shape, std::vector<int> &out_shape) override;
		virtual int allocBuf4Node(const std::vector<int> &in_shape,
			const std::vector<int> &out_shape,
			std::vector<std::shared_ptr<Tensor<Dtype>>> &data) const override;
		virtual int allocOpBuf4Train(const std::vector<int> &in_shape, const std::vector<int> &out_shape) override;
		virtual void forward(const std::vector<std::shared_ptr<Tensor<Dtype>>> &prev, const std::vector<std::shared_ptr<Tensor<Dtype>>> &next) override;
		virtual void backward(const std::vector<std::shared_ptr<Tensor<Dtype>>> &prev, const std::vector<std::shared_ptr<Tensor<Dtype>>> &next,
			const std::vector<std::shared_ptr<Tensor<Dtype>>> &prev_diff, const std::vector<std::shared_ptr<Tensor<Dtype>>> &next_diff) override;

	private:
		std::string op_type_;
		SoftmaxCrossEntropyLossHOpParam param_;
		std::vector<std::shared_ptr<Tensor<Dtype>>> diff_;
		//std::vector<std::shared_ptr<Op<Dtype>>> sub_ops_;
	};

	/////////////////////////////  Creator   /////////////////////////////////

	template <typename Dtype>
	SoftmaxCrossEntropyLossHOp<Dtype> *CreateOp(SoftmaxCrossEntropyLossHOpParam &param)
	{
		SoftmaxCrossEntropyLossHOp<Dtype> *op = NULL;
		op = new SoftmaxCrossEntropyLossHOp<Dtype>(param);
		return op;
	}

	template <typename Dtype>
	std::shared_ptr<Op<Dtype>> CreateSoftmaxCrossEntropyLossHOp()
	{
		return std::shared_ptr<Op<Dtype>>(new SoftmaxCrossEntropyLossHOp<Dtype>());
	}
}
#endif