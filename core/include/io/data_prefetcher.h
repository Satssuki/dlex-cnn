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
	// ��ȡmnist���ݣ�pushdata���ж�ģʽ����ΪGPU���뵽GPU���У���input_op��prefetcher��Ϊ������뵽input��prefetcher����network����pushData����һ��
	// block_queue
	public:
		int pushData(std::shared_ptr<dlex_cnn::Tensor<Dtype>> input_data,
			std::shared_ptr<dlex_cnn::Tensor<Dtype>> label_data);
		int popData();

	protected:
		void entryInnerThread();

	private:
		std::queue < std::pair < std::shared_ptr<dlex_cnn::Tensor<float>>, std::shared_ptr<dlex_cnn::Tensor<float>> > > data_queue_;
		std::vector<float> mean_value_;
	};
}

#endif //DLEX_PREFETCH_HPP_