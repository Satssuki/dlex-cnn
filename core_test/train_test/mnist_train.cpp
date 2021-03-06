////////////////////////////////////////////////////////////////
// > Copyright (c) 2017 by Contributors. 
// > https://github.com/cjmcv
// > brief  Train with mnist data.
// > author Jianming Chen
////////////////////////////////////////////////////////////////

#include "mnist_train.h"

namespace dlex_cnn {
MnistTrainTest::MnistTrainTest() {
  train_images_file_ = "../../../res/mnist_data/Train-images.idx3-ubyte";
  train_labels_file_ = "../../../res/mnist_data/Train-labels.idx1-ubyte";

  test_images_file_ = "../../../res/mnist_data/t10k-images.idx3-ubyte";
  test_labels_file_ = "../../../res/mnist_data/t10k-labels.idx1-ubyte";

  model_saved_path_ = "./";

  // Super parameters
  batch_size_ = 128;
  learning_rate_ = 0.1f;
  decay_rate_ = 0.8f;
  min_learning_rate_ = 0.001f;
  test_after_batches_ = 50;
  max_batches_ = 10000;
  lr_setp_ = 200;
  save_iter_ = 19;

  printf("MnistTrainTest constructed.\n");
}

bool MnistTrainTest::LoadMnistData(tind::Phase phase) {
  // Set super parameter = tind::Train;
  MnistLoader loader;

  float crossVal;
  std::string images_file, labels_file;
  if (phase == tind::Train) {
    images_file = train_images_file_;
    labels_file = train_labels_file_;

    crossVal = 0.9;
  }
  else {
    images_file = test_images_file_;
    labels_file = test_labels_file_;

    crossVal = 0.0;
  }

  bool success = false;
  //load Train images
  printf("loading training data...\n");
  std::vector<dlex_cnn::IMAGE_DATUM> images;
  success = loader.LoadMnistImages(images_file, images);
  assert(success && images.size() > 0);
  //load Train labels
  std::vector<char> labels;
  success = loader.LoadMnistLabels(labels_file, labels);
  assert(success && labels.size() > 0);
  assert(images.size() == labels.size());

  // train_data_
  for (int i = 0; i < images.size()*crossVal; i++)
    train_data_.push_back(std::make_pair(images[i], labels[i]));
  std::random_shuffle(train_data_.begin(), train_data_.end());

  // validate_data_
  for (int i = images.size()*crossVal; i < images.size(); i++)
    validate_data_.push_back(std::make_pair(images[i], labels[i]));
  std::random_shuffle(validate_data_.begin(), validate_data_.end());

  return true;
}

bool MnistTrainTest::ReleaseMnistData() {
  for (int i = 0; i < train_data_.size(); i++)
    delete train_data_[i].first.pdata;

  for (int i = 0; i < validate_data_.size(); i++)
    delete validate_data_[i].first.pdata;

  return true;
}

bool MnistTrainTest::FetchBatchData(
  const std::vector<std::pair<dlex_cnn::IMAGE_DATUM, char>> &train_data_,
  std::shared_ptr<dlex_cnn::Tensor<float>> input_data_tensor,
  std::shared_ptr<dlex_cnn::Tensor<float>> label_data_tensor,
  const int offset, const int length) {
  assert(input_data_tensor->get_shape()[0] == label_data_tensor->get_shape()[0]);
  if (offset >= train_data_.size()) {
    return false;
  }
  int actual_end_pos = offset + length;
  if (actual_end_pos > train_data_.size()) {
    return false;
    ////image data
    ////auto inputDataSize = input_data_tensor->get_size();
    //input_data_tensor->get_shape()[0] = train_data_.size() - offset;
    //actual_end_pos = offset + input_data_tensor->get_shape()[0];
    //input_data_tensor.reset(new dlex_cnn::Tensor<float>(input_data_tensor->get_shape()));
    ////label data
    ////auto labelDataSize = label_data_tensor->get_size();
    //label_data_tensor->get_shape()[0] = input_data_tensor->get_shape()[0];
    //label_data_tensor.reset(new dlex_cnn::Tensor<float>(label_data_tensor->get_shape()));
  }
  //printf("ready to copy\n");
  //copy
  const int size_per_image = input_data_tensor->get_size()[dlex_cnn::tind::e3D];
  const int size_per_label = label_data_tensor->get_size()[dlex_cnn::tind::e3D];
  assert(size_per_image == train_data_[0].first.channels*train_data_[0].first.width*train_data_[0].first.height);
  //scale to 0.0f~1.0f
  const float scale_rate = 1.0f / 255.0f;
  for (int i = offset; i < actual_end_pos; i++) {
    //image data
    float* input_data = (float *)input_data_tensor->GetPushCpuData() + (i - offset)*size_per_image;//*sizeof(float)
    const uint8_t* image_data = train_data_[i].first.pdata;
    for (int j = 0; j < size_per_image; j++) {
      input_data[j] = (float)image_data[j] * scale_rate;
    }
    //label data
    float* label_data = (float *)label_data_tensor->GetPushCpuData() + (i - offset)*size_per_label;//*sizeof(float)
    const uint8_t label = train_data_[i].second;
    for (int j = 0; j < size_per_label; j++) {
      label_data[j] = label;
    }
  }
  //printf("finish fetch\n");
  return true;
}

std::shared_ptr<dlex_cnn::Tensor<float>> MnistTrainTest::ConvertLabelToTensor(
  const std::vector< std::pair<dlex_cnn::IMAGE_DATUM, char> > &test_data, 
  const int start, const int len) {
  assert(test_data.size() > 0);
  const int number = len;
  const int size_per_label = 1;

  std::shared_ptr<dlex_cnn::Tensor<float>> result(new dlex_cnn::Tensor<float>(number, size_per_label, 1, 1));
  for (int i = start; i < start + len; i++) {
    float* label_data = (float *)result->GetPushCpuData() + (i - start)*size_per_label;//*sizeof(float)
    const uint8_t label = test_data[i].second;
    for (int j = 0; j < size_per_label; j++) {
      label_data[j] = label;
    }
  }
  return result;
}

std::shared_ptr<dlex_cnn::Tensor<float>> MnistTrainTest::ConvertVectorToTensor(
  const std::vector< std::pair<dlex_cnn::IMAGE_DATUM, char> > &test_data,
  const int start, const int len) {
  assert(test_data.size() > 0);
  const int number = len;
  const int channel = test_data[0].first.channels;
  const int width = test_data[0].first.width;
  const int height = test_data[0].first.height;
  const int size_per_image = channel*width*height;
  const float scale_rate = 1.0f / 255.0f;
  //std::shared_ptr<dlex_cnn::DataTensor> result(new dlex_cnn::DataTensor(dlex_cnn::DataSize(number, channel, width, height)));
  std::shared_ptr<dlex_cnn::Tensor<float>> result(new dlex_cnn::Tensor<float>(number, channel, height, width));
  for (int i = start; i < start + len; i++) {
    //image data
    float* input_data = (float *)result->GetPushCpuData() + (i - start)*size_per_image;
    const uint8_t* image_data = test_data[i].first.pdata;
    for (int j = 0; j < size_per_image; j++) {
      input_data[j] = (float)image_data[j] * scale_rate;
    }
  }
  return result;
}

uint8_t MnistTrainTest::getMaxIdxInArray(const float* start, const float* stop) {
  assert(start && stop && stop >= start);
  int result = 0;
  int len = stop - start;
  for (int i = 0; i < len; i++)
    if (start[i] >= start[result])
      result = i;

  return (uint8_t)result;
}

std::pair<float, float> MnistTrainTest::testInTrain(dlex_cnn::NetWork<float>& network,
  const int batch, 
  const std::vector< std::pair<dlex_cnn::IMAGE_DATUM, char> > &test_data) {
  //printf("into Test: batch = %d, %d, %d\n", batch, test_labels.size(), test_images.size());
  assert(test_data.size() > 0);
  int correct_count = 0;
  float loss = 0.0f;
  int batchs = 0;
  //test_data.size()
  for (int i = 0; i < test_data.size(); i += batch, batchs++) {
    const int start = i;
    int t_label_size = test_data.size();
    const int len = std::min(t_label_size - start, batch);
    //printf("len = %d\n", len);
    const std::shared_ptr<dlex_cnn::Tensor<float>> input_data_tensor = ConvertVectorToTensor(test_data, start, len);	//data -> test_data.first
    const std::shared_ptr<dlex_cnn::Tensor<float>> label_data_tensor = ConvertLabelToTensor(test_data, start, len);	//label -> test_data.second

    network.TestBatch(input_data_tensor, label_data_tensor);//
    std::shared_ptr<dlex_cnn::Tensor<float>> probDataTensor;
    network.GetNodeData("output", probDataTensor);

    //float *pdata = (float *)probDataTensor->GetPushCpuData();
    //for (int i = 0; i < probDataTensor->get_size()[dlex_cnn::tind::e4D]; i++)
    //{
    //	printf("%f, ", pdata[i]);
    //}
    //network.graph_->GetNodeData("output", std::vector<std::shared_ptr<Tensor<Dtype>>> &cpuData);

    //printf("ready to get loss\n");
    ////get loss
    //const float batch_loss = 0; // network.GetLoss(label_data_tensor, probDataTensor);
    //loss = dlex_cnn::moving_average(loss, batchs + 1, batch_loss);
    //printf("loss = %f\n", loss);
    ////printf("loss: %f, %f\n", batch_loss, loss);

    //printf("ready to get acc\n");
    const int label_size = probDataTensor->get_size()[dlex_cnn::tind::e3D];
    const float* prob_data = (float *)probDataTensor->GetPushCpuData();
    for (int j = 0; j < len; j++) {
      const uint8_t real_prob = test_data[i + j].second;
      const uint8_t test_prob = getMaxIdxInArray(prob_data + j*label_size, prob_data + (j + 1) * label_size);
      if (real_prob == test_prob) {
        correct_count++;
      }
    }
    //printf("finish\n");
  }

  const float accuracy = (float)correct_count / (float)test_data.size();
  return std::pair<float, float>(accuracy, 0);	//loss
}

bool MnistTrainTest::LoadBatch(void *instant_ptr, 
  std::pair < std::shared_ptr<Tensor<float>>, 
  std::shared_ptr<Tensor<float>> > *tensor_pair) {
  MnistTrainTest *instant = (MnistTrainTest *)instant_ptr;

  if (tensor_pair->first == NULL || tensor_pair->first->get_size()[tind::e4D] != instant->data_size_4D_) {
    tensor_pair->first.reset(new dlex_cnn::Tensor<float>(instant->batch_size_, instant->channels_, instant->height_, instant->width_));
    tensor_pair->second.reset(new dlex_cnn::Tensor<float>(instant->batch_size_, 1, 1, 1));
    printf("LoadBatch -> reset tensor_pair\n");
  }

  if (!instant->FetchBatchData(instant->train_data_, 
    tensor_pair->first, 
    tensor_pair->second, 
    instant->prefetch_batch_idx_ * instant->batch_size_, 
    instant->batch_size_)) {
    instant->prefetch_batch_idx_ = 0;
    std::random_shuffle(instant->train_data_.begin(), instant->train_data_.end());
    //epoch_idx++;	// epoch 改为外面计算，用全部数据得到一个epoch有多少个batch - to do
    false;
  }
  instant->prefetch_batch_idx_++;

  return true;
}

void MnistTrainTest::StartPrefetchData(NetWork<float> &network) {
  prefetch_batch_idx_ = 0;
  network.prefetcher_.SetInstantiation(this);
  network.prefetcher_.batch_loader_pfunc_ = LoadBatch;

  network.prefetcher_.StartInnerThread();
}

void MnistTrainTest::TrainWithPrefetcher() {
  // Data preparing.
  if (!LoadMnistData(tind::Train)) {
    printf("error LoadMnistData\n");
    return;
  }
  class_num_ = 10;
  channels_ = train_data_[0].first.channels;
  width_ = train_data_[0].first.width;
  height_ = train_data_[0].first.height;
  data_size_4D_ = batch_size_ * channels_ * width_ * height_;

  RegisterOpClass();
  Task::set_mode(tind::Mode::GPU);

  NetWork<float> network("netA");

  bool is_use_pretrain = false;
  if (is_use_pretrain) {
    network.LoadStageModel("./", 1);
  }
  else {
    TypicalNet typicalNet;
    typicalNet.lenet<float>(batch_size_, channels_, height_, width_, network);
    network.NetParamsInit();
  }
  printf("Finish loading network structure.\n");

  StartPrefetchData(network);

  std::vector<std::string> in_node_names;
  in_node_names.push_back("input");
  std::vector<std::string> out_node_names;
  out_node_names.push_back("output");
  network.SetIONodeName(in_node_names, out_node_names);

  float val_accuracy = 0.0f;
  float train_total_loss = 0.0f;
  int train_batches = 0;
  float val_loss = 0.0f;
  int epoch_idx = 0;

  network.SetLearningRate(learning_rate_);
  for (int batch_idx = 0; batch_idx < max_batches_; batch_idx++) {
    const float batch_loss = network.TrainBatch();

    train_batches++;
    train_total_loss += batch_loss;
    printf("batch[%d]->train_batch_loss: %f, learning_rate: %f\n", batch_idx, batch_loss, learning_rate_);
    if (batch_idx && batch_idx % test_after_batches_ == 0) {
      //network.SwitchPhase(dlex_cnn::tind::Phase::Test);
      std::tie(val_accuracy, val_loss) = testInTrain(network, 128, validate_data_);	// 要注意最后一个batch为112，会修改掉所有节点的num维度，在下一轮训练过程中修改回来。
      printf("sample : %d/%zd , learning_rate : %f , train_avg_loss : %f , val_loss : %f , val_accuracy : %.4f%%\n",
        batch_idx*batch_size_, train_data_.size(), learning_rate_, train_total_loss / train_batches, val_loss, val_accuracy*100.0f);

      train_total_loss = 0.0f;
      train_batches = 0;
      //network.SwitchPhase(dlex_cnn::tind::Phase::Train);
    }
    if (batch_idx && batch_idx % save_iter_ == 0)
      network.SaveStageModel(model_saved_path_, 1);

    if (batch_idx && batch_idx % lr_setp_ == 0) {
      //update learning rate
      learning_rate_ = std::max(learning_rate_*decay_rate_, min_learning_rate_);
      network.SetLearningRate(learning_rate_);
    }
  }
}

// Not fix, will be merged with TrainWithPrefetcher
void MnistTrainTest::Train() {
  Task::set_mode(tind::Mode::GPU);
  NetWork<float> network("netA");

  //StartPrefetchData(network);

  if (!LoadMnistData(tind::Train)) {
    printf("error LoadMnistData\n");
    return;
  }

  float learning_rate = 0.1f;
  const float decay_rate = 0.8f;
  const float min_learning_rate = 0.001f;
  const int test_after_batches = 60;
  const int max_batches = 10000;
  int batch_size = 128;
  const int channels = train_data_[0].first.channels;
  const int width = train_data_[0].first.width;
  const int height = train_data_[0].first.height;
  printf("test_after_batches:%d\n", test_after_batches);
  printf("learning_rate:%f ,decay_rate:%f , min_learning_rate:%f\n", learning_rate, decay_rate, min_learning_rate);
  printf("channels:%d , width:%d , height:%d\n", channels, width, height);

  printf("construct network begin...\n");
  RegisterOpClass();

  TypicalNet typicalNet;
  typicalNet.mix<float>(batch_size, channels, height, width, network);

  std::vector<std::string> in_node_names;
  in_node_names.push_back("input");
  std::vector<std::string> out_node_names;
  out_node_names.push_back("output");
  network.SetIONodeName(in_node_names, out_node_names);

  network.SetLearningRate(learning_rate);

  //dlex_cnn::NetWork<float> network;
  //network.netWorkInit("netA");
  //network.LoadStageModel("./", 1);

  printf("construct network done.\n");

  float val_accuracy = 0.0f;
  float train_total_loss = 0.0f;
  int train_batches = 0;
  float val_loss = 0.0f;

  //Train
  printf("begin training...\n");
  std::shared_ptr<dlex_cnn::Tensor<float>> input_data_tensor = std::make_shared<dlex_cnn::Tensor<float>>(batch_size, channels, height, width);	//shoule be moved
  std::shared_ptr<dlex_cnn::Tensor<float>> label_data_tensor = std::make_shared<dlex_cnn::Tensor<float>>(batch_size, 1, 1, 1);//class_num_

  int save_iter = 19;
  std::string model_saved_path = "./";

  int lr_setp = 200;
  int epoch_idx = 0;

  //before epoch start, shuffle all Train data first
  batch_size = 128;	//shoule be changed
  int fetch_off_set = 0;
  for (int batch_idx = 0; batch_idx < max_batches; batch_idx++) {
    //batch_size++;
    printf("batch size = %d\n", batch_size);
    if (input_data_tensor->get_shape()[tind::eNum] != batch_size) {
      input_data_tensor.reset(new dlex_cnn::Tensor<float>(batch_size, channels, height, width));
      label_data_tensor.reset(new dlex_cnn::Tensor<float>(batch_size, 1, 1, 1));
    }
    printf("start Train batch : %d\n", batch_idx);
    if (!FetchBatchData(train_data_, input_data_tensor, label_data_tensor, fetch_off_set*batch_size, batch_size)) {
      fetch_off_set = 0;
      std::random_shuffle(train_data_.begin(), train_data_.end());
      epoch_idx++;
      continue;
    }
    else
      fetch_off_set++;

    printf("finish FetchBatchData\n");

    //network.NetWorkShow();

    const float batch_loss = network.TrainBatch(input_data_tensor, label_data_tensor);

    train_batches++;
    train_total_loss += batch_loss;
    printf("batch[%d]->train_batch_loss: %f, learning_rate: %f\n", batch_idx, batch_loss, learning_rate);
    if (batch_idx && batch_idx % test_after_batches == 0) {
      network.SwitchPhase(dlex_cnn::tind::Phase::Test);
      std::tie(val_accuracy, val_loss) = testInTrain(network, 128, validate_data_);	// 要注意最后一个batch为112，会修改掉所有节点的num维度，在下一轮训练过程中修改回来。
      printf("sample : %d/%d , learning_rate : %f , train_avg_loss : %f , val_loss : %f , val_accuracy : %.4f%%\n",
        batch_idx*batch_size, train_data_.size(), learning_rate, train_total_loss / train_batches, val_loss, val_accuracy*100.0f);

      train_total_loss = 0.0f;
      train_batches = 0;
      network.SwitchPhase(dlex_cnn::tind::Phase::Train);
    }
    if (batch_idx && batch_idx % save_iter == 0)
      network.SaveStageModel(model_saved_path, 1);

    if (batch_idx && batch_idx % lr_setp == 0) {
      //network.SwitchPhase(dlex_cnn::tind::Phase::Test);
      //std::tie(val_accuracy, val_loss) = testInTrain(network, 128, validate_data_);
      //network.SwitchPhase(dlex_cnn::tind::Phase::Train);

      //update learning rate
      learning_rate = std::max(learning_rate*decay_rate, min_learning_rate);
      network.SetLearningRate(learning_rate);
      //printf("epoch[%d] val_loss : %f , val_accuracy : %.4f%%\n", epoch_idx++, val_loss, val_accuracy*100.0f);
    }
  }

  std::tie(val_accuracy, val_loss) = testInTrain(network, 128, validate_data_);
  printf("final val_loss : %f , final val_accuracy : %.4f%%\n", val_loss, val_accuracy*100.0f);
  //success = network.saveModel(modelFilePath);
  //assert(success);
  printf("finished training.\n");
  ReleaseMnistData();
}

void MnistTrainTest::Test(const std::string& modelFilePath, const int iter) {
  Task::set_mode(tind::Mode::CPU);

  LoadMnistData(tind::Test);

  const int batch = 128;
  const int channels = validate_data_[0].first.channels;
  const int width = validate_data_[0].first.width;
  const int height = validate_data_[0].first.height;
  printf("channels:%d , width:%d , height:%d\n", channels, width, height);

  RegisterOpClass();

  printf("construct network begin...\n");
  dlex_cnn::NetWork<float> network("netA");
  network.LoadStageModel(modelFilePath, iter);
  printf("construct network done.\n");

  std::vector<std::string> in_node_names;
  in_node_names.push_back("input");
  std::vector<std::string> out_node_names;
  out_node_names.push_back("output");
  network.SetIONodeName(in_node_names, out_node_names);

  //Test
  printf("begin Test...\n");
  float accuracy = 0.0f, loss = std::numeric_limits<float>::max();
  std::tie(accuracy, loss) = testInTrain(network, batch, validate_data_);
  printf("accuracy : %.4f%%\n", accuracy*100.0f);
  printf("finished Test.\n");

  //// free
  ReleaseMnistData();
}
}

void MnistTrain() {
  dlex_cnn::MnistTrainTest mnist;
  //mnist.Train();
  mnist.TrainWithPrefetcher();
  system("pause");

  return;
}

void MnistTest() {
  dlex_cnn::MnistTrainTest mnist;
  const std::string model_file = "./";
  mnist.Test(model_file, 1);
  system("pause");

  return;
}
