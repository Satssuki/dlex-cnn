////////////////////////////////////////////////////////////////
// > Copyright (c) 2017 by Contributors. 
// > https://github.com/cjmcv
// > brief  
// > author Jianming Chen
////////////////////////////////////////////////////////////////

// ���߳�����Ԥ��, batch������ʹ���ڴ��
#ifndef DLEX_PREFETCH_HPP_
#define DLEX_PREFETCH_HPP_

#include "util/thread_pool.h"
#include "tensor.h"

#include <queue>

namespace dlex_cnn
{
	template <typename Dtype>
	class DataPrefetcher
	{
	public:
		DataPrefetcher();
		virtual ~DataPrefetcher();

	// �豸��Ϣ���ȫ�֣�prefetch��input_opȡ�����ⲿ����У�������һ�߳��Լ�������
	// ��ȡmnist���ݣ�pushdata���ж�ģʽ����ΪGPU���뵽GPU���У���input_op������prefetcher��prefetcher����network����pushData����һ��
	public:
		int pushData(std::shared_ptr<dlex_cnn::Tensor<float>> input_data, 
					 std::shared_ptr<dlex_cnn::Tensor<float>> label_data);
		int popData();

	private:
		std::queue<>
		std::vector<float> mean_value_;
	};
}

#endif //DLEX_PREFETCH_HPP_