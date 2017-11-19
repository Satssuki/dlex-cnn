////////////////////////////////////////////////////////////////
// > Copyright (c) 2017 by Contributors. 
// > https://github.com/cjmcv
// > brief  
// > author Jianming Chen
////////////////////////////////////////////////////////////////

#ifndef DLEX_TIMER_HPP_
#define DLEX_TIMER_HPP_

#include <chrono>

// finish
// DLOG_ERR("Convolution load_param failed\n");�������жϣ�������־����linuxѵ���ɼ��ض����ض���ʱ��stderr�Ի���ʾ���նˣ�������ȥ�ļ���
// ��Ա�����淶��������Сд������_Сд��, train/test��Ҫ�ɹ���
// ���graph ��node size��ӡ,graph_.reset(new Graph<Dtype>())��Ҫ���ڹ�����-->�ѷ���init
// ���opParam��ӳ����ִ�������Ҫ���ַ����õ�opParam -> 
// ��node��һ���ַ������ڱ�ʾ��ӦopParam��
// ģ�Ͷ�д��1��������ģ�ͣ������ӣ��ṹ+blob������optimizer�ȣ��������ʹ�ã���-- �Ӻ�
//			 2��������ӽṹ + ���ӳ����� + ������blob�ȣ� ���ڴ洢�׶�ѵ��ģ�͡�  --  ���
// �ַ���if else����֧������Ӧ���󷵻�
// дʱ����ִ��param���ִ�������op��дʵ�ַ���string������nodeд������ʱ����param�ִ���ֵ��node�У���nodeͨ��inte_op��д��param���ִ���
// op������Ĭ�Ϲ��죬ʹ�ò���ʱҪ����ж��Ƿ������úò���

// ��ģ�Ͷ�д->���node���ַ�������
// tensor�У�num,channels, height, width����height/width��˳����Ҫ��opencvͼ�������Ӧȷ�ϣ���ȷ��


// ���������⣺
// Convolution group���δʵ��
// ��conv��deconv�Ĳ�����kernel num/channels �ĳ�output num����ͳһ?�����Ϊoutput num�����ڻ���num/channel
// Pooling op�м���ֵ�ػ�max_idx_map_, �ڲ�ѵ��ʱ����ʡ����
// �����������м�����õĽڵ�������
// optimizer�޸��Ż�
// op���inline�ͷ���const


// network�������������Ԥ�������һ���;�ֵ

// (�Ż�)�淶���������� node�н϶ຯ��ֱ�ӵ���op�ģ�
// ���ͳһ������


// ֻ���Բ�ѵ��ʱ������ҪΪdiff_��gradient_�����ڴ�
// softmax_cross_entropy_hop.cpp ��allocOpBuf4Train����op�Ĵ�����Ҫ�ģ��ĵ�һ��ӳ�䣿


// ģ�ͼ���(�Ӻ���)�������̣߳���Ӷ�·����������ڵ㣬���ڵ�������ж�/����ͼģ�����棩
// �߳����ڴ����
// ���ע��
// �Ժ궨����ӵ�������־�⣿��


// node��data��w/b�Ƿ���Ҫ����--�ݲ�����

// ����һЩnode����һЩ�������򵥴�������Ӱ���������ӣ���caffe�е�relu�ȣ���������ֱ��ɾ���Կ��Լ���ʹ�ã�������ֽڵ����Ӳ��ϵ�����
// ���Կ��Ǽ�һ�������node����Ӱ��������������ӣ�����֧�ķ�ʽ���룬���������������ݴ���ԭ�ط������ݣ�������Ӳ�����
// ���network init����graph��new�Ƶ�network��init��

// network���� ����graph����������ѵ�����Կ��ƣ�optimizer��
// snapshot��lr��optimizer��test_iter, max_iter��CPU/GPU
// lr���ƺ�������net���棬���õ�optimizer�У�

namespace dlex_cnn
{
class Timer {
public:
	typedef std::chrono::high_resolution_clock clock;
	typedef std::chrono::nanoseconds ns;
	Timer() { Start(); }
	/**
	* @brief Starts a timer.
	*/
	inline void Start() { start_time_ = clock::now(); }
	inline float NanoSeconds() {
		return std::chrono::duration_cast<ns>(clock::now() - start_time_).count();
	}
	/**
	* @brief Returns the elapsed time in milliseconds.
	*/
	inline float MilliSeconds() { return NanoSeconds() / 1000000.f; }
	/**
	* @brief Returns the elapsed time in microseconds.
	*/
	inline float MicroSeconds() { return NanoSeconds() / 1000.f; }
	/**
	* @brief Returns the elapsed time in seconds.
	*/
	inline float Seconds() { return NanoSeconds() / 1000000000.f; }

protected:
	std::chrono::time_point<clock> start_time_;
};
}
#endif //DLEX_TIMER_HPP_