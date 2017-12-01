////////////////////////////////////////////////////////////////
// > Copyright (c) 2017 by Contributors. 
// > https://github.com/cjmcv
// > brief  
// > author Jianming Chen
////////////////////////////////////////////////////////////////

// ���߳�����Ԥ��, batch������ʹ���ڴ��
#ifndef DLEX_PREFETCH_HPP_
#define DLEX_PREFETCH_HPP_

#include "tensor.h"
#include "util/thread_inner.h"
#include "util/blocking_queue.h"

#include <queue>

namespace dlex_cnn
{
	template <typename Dtype>
	class DataPrefetcher : ThreadInner
	{
		using TensorPair = std::pair < std::shared_ptr<Tensor<Dtype>>, std::shared_ptr<Tensor<Dtype>> >;
	public:
		DataPrefetcher();
		virtual ~DataPrefetcher();

	// �豸��Ϣ���ȫ�֣�prefetch��input_opȡ�����ⲿ����У�������һ�߳��Լ�������
	// ��ȡmnist���ݣ�pushdata���ж�ģʽ����ΪGPU���뵽GPU���У���input_op��prefetcher��Ϊ������뵽input��prefetcher����network����pushData����һ��
	// block_queue
	public:
		static const int PREFETCH_COUNT = 3;

		inline void setInstantiation(void *ptr) { instant_ = ptr; }
		void *instant_ = NULL;
		bool(*batch_loader_pfunc_)(void *, TensorPair*) = NULL;
		bool loadBatch(TensorPair* batch);

		virtual void entryInnerThread();
	private:

		TensorPair base_storage_[PREFETCH_COUNT];
		BlockingQueue < TensorPair* > free_;
		BlockingQueue < TensorPair* > full_;
	};
}

#endif //DLEX_PREFETCH_HPP_