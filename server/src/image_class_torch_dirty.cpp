/// this has manual hard-coded implementations of some of the operations using torch models... need to refactor.


std::vector<float> random_input_generator(int min_value = 1,
                                          int max_value = 100,
                                          size_t vector_size = 150528,
                                          bool is_print = true) {
    std::random_device rd;
    std::mt19937 rng(rd());

    std::uniform_int_distribution<int> distribution(min_value, max_value);

    std::vector<float> res_data(vector_size);
    for (size_t i = 0; i < vector_size; ++i) {
        res_data[i] = distribution(rng);
    }

    if (is_print) {
        // Print the vector contents
        std::cout << "The first Random numbers:";
        for (int value : res_data) {
            std::cout << " " << value;
            break;
        }
        std::cout << std::endl;
    }
    return res_data;
}


void convertTorchTensor(const vaccel::TorchTensor &src,
                        struct vaccel_torch_tensor &dst) {
    dst.data = const_cast<char *>(src.data().data());
    dst.size = src.data().size();
    dst.dims = const_cast<int32_t *>(
        reinterpret_cast<const int32_t *>(src.dims().data()));
    dst.nr_dims = src.dims_size();
}

auto vaccel_preprocess(std::string text, std::map<std::string, int> token2id, int max_length, bool log = false) {
    std::string pad_token = "[PAD]", start_token = "[CLS]", end_token = "[SEP]";
    int pad_token_id = token2id[pad_token], start_token_id = token2id[start_token], end_token_id = token2id[end_token];

    std::vector<int> input_ids(max_length, pad_token_id), masks(max_length, 0);
    input_ids[0] = start_token_id; masks[0] = 1;

    std::string word;
    std::istringstream ss(text);

    int input_id = 1;
    while(getline(ss, word, ' ')) {
        int word_id = token2id[word];
        masks[input_id] = 1;
        input_ids[input_id++] = word_id;
        
        if (log)
            std::cout << word << " : " << word_id << '\n';
    }

    masks[input_id] = 1;
    input_ids[input_id] = end_token_id;

    if (log){
        for (auto i : input_ids)
            std::cout << i << ' ';
        std::cout << '\n';
    
        for (auto i : masks)
            std::cout << i << ' ';
        std::cout << '\n';
    }

    std::vector<std::vector<int>> input_ids_tensor = {input_ids};
    std::vector<std::vector<int>> first_unsqueeze = {masks};
    std::vector<std::vector<std::vector<int>>> masks_tensor = {first_unsqueeze};
    
    return std::make_pair(input_ids_tensor, masks_tensor);
}

std::map<std::string, int> createToken2Id() {
    std::map<std::string, int> token2id;

    token2id["[PAD]"] = 0;
    token2id["[CLS]"] = 1;
    token2id["[SEP]"] = 2;
    token2id["hello"] = 3;
    token2id["world"] = 4;
    token2id["this"] = 5;
    token2id["is"] = 6;
    token2id["a"] = 7;
    token2id["test"] = 8;

    return token2id;
}

grpc::Status ServiceImpl::TorchJitloadForward(
    ::grpc::ServerContext *context,
    const ::vaccel::TorchJitloadForwardRequest *request,
    ::vaccel::TorchJitloadForwardResponse *response) {
    printf("Received TorchJitloadForward request with model ID: %ld\n",
           request->model_id());
    int session_id = request->session_id();
    int model_id = request->model_id();
    std::string bytes = request->run_options();

    int tensor_count = request->in_tensors_size();

    std::vector<vaccel::TorchTensor> tensor_vector;
    for (int i = 0; i < tensor_count; ++i) {
        tensor_vector.push_back(request->in_tensors(i));
    }

    std::vector<vaccel_torch_tensor> converted_tensors(tensor_count);
    std::vector<vaccel_torch_tensor *> tensor_ptrs(tensor_count);

    for (int i = 0; i < tensor_count; ++i) {
        convertTorchTensor(tensor_vector[i], converted_tensors[i]);
        tensor_ptrs[i] = &converted_tensors[i];
    }

    auto it = sessions_map.find(session_id);
    if (it == sessions_map.end()) {
        return grpc::Status(grpc::StatusCode::NOT_FOUND,
                            "Session ID not found");
    }

    struct vaccel_session *sess_ptr = &(it->second);
    struct vaccel_torch_saved_model model;
    // std::string run_options_str = "resnet";
    struct vaccel_torch_buffer run_options = {0};
    // run_options.data = const_cast<char*>(run_options_str.c_str());
    // run_options.size = run_options_str.size();
    int ret;



    /// bert example /// 

    // // const char *model_path = "/home/jl/vaccel-torch-bert-example/";
    

    // // ret = vaccel_torch_saved_model_set_path(&model, model_path);
    // // if (ret != 0) {
    // //     return grpc::Status(grpc::StatusCode::INTERNAL,
    // //                         "Failed to set model path");
    // // }


    // // ret = vaccel_torch_saved_model_register(&model);
    // // if (ret != 0) {
    // //     return grpc::Status(grpc::StatusCode::INTERNAL,
    // //                         "Failed to register model");
    // // }


    // // struct vaccel_torch_tensor *in;
    // // struct vaccel_torch_tensor *out;
    // // int64_t dims[] = {1, 224, 224, 3};
    // // std::vector<float> res_data = random_input_generator();
    // // res_data.resize(3 * 224 * 224);



    // // ret = vaccel_sess_register(sess_ptr, model.resource);
    // // if (ret) {
    // //     fprintf(stderr, "Could not register model with session\n");
    // // }


    // // in = vaccel_torch_tensor_new(4, dims, VACCEL_TORCH_FLOAT);
    // // if (!in) {
    // //     fprintf(stderr, "Could not allocate memory\n");
    // // }


    // // in->data = res_data.data();
    // // in->size = res_data.size() * sizeof(float);
    // // std::cout << "The IN ADDRESS: " << in->data << std::endl;
    // // std::cout << "size: " << in->size << std::endl;


    // // /* Output tensor */
    // // out = (struct vaccel_torch_tensor *)malloc(
    // //     sizeof(struct vaccel_torch_tensor) * 1);


    // float *offsets;
    // int64_t dims_id[] = { 1, 32};
    // int64_t dims_mask[] = { 1, 1, 32};

    // // in[0] =  input_ids; in[1] = masks;
    // struct vaccel_torch_tensor *in[2];
    // struct vaccel_torch_tensor *out;

    // ret = vaccel_torch_saved_model_set_path (&model, model_path);

    // if (ret) {
    //     vaccel_debug("Could not set model path to Torch model");
    //     exit(1);
    // }

    // vaccel_debug("Created new model %lld\n", vaccel_torch_saved_model_id (&model));

    // ret = vaccel_torch_saved_model_register(&model);
    // if (ret != 0) {
    //     return grpc::Status(grpc::StatusCode::INTERNAL,
    //                         "Failed to register model");
    // }

    // vaccel_debug("Registered model %lld\n", vaccel_torch_saved_model_id (&model));

    // ret = vaccel_sess_register (sess_ptr, model.resource);
    // if (ret) {
    //     fprintf (stderr, "Could not register model with session\n");
    //     exit (1);
    // }

    // vaccel_debug("Registered model ID with session pointer");


    // in[0] = vaccel_torch_tensor_new(2, dims_id, VACCEL_TORCH_FLOAT);
    // in[1] = vaccel_torch_tensor_new(3, dims_mask, VACCEL_TORCH_FLOAT);

    // vaccel_debug("Created input tensors");


    // std::vector<std::vector<int>> input_ids;
    // std::vector<std::vector<std::vector<int>>> masks;

    // std::string text = "hello world";
    // std::map<std::string, int> token2id = createToken2Id();
    // int max_length = 1;

    // std::tie(input_ids, masks) = vaccel_preprocess(text, token2id, max_length);

    // in[0]->data = input_ids.data()->data();

    // vaccel_debug("test1");
    // in[1]->data = masks.data()->data()->data();
    // vaccel_debug("test1");

    // in[0]->size = input_ids.data()->size() * sizeof(int);
    // vaccel_debug("test1");
    // in[1]->size = masks.data()->data()->size() * sizeof(int);
    // vaccel_debug("test1");

    // out = (struct vaccel_torch_tensor *) malloc (sizeof (struct vaccel_torch_tensor) * 1);

    // vaccel_debug("Created output tensors");

    // ret = vaccel_torch_jitload_forward(sess_ptr, &model, &run_options, in, 2, &out, 1);




    /// cv example /// 

    const char *model_path = "/home/jl/vaccel-torch-cv-example/";
    

    float *offsets;
    int64_t dims_id[] = { 1, 32};
    int64_t dims_mask[] = { 1, 1, 32};

    std::string run_options_str = "resnet";
    run_options.data = (char*) malloc(sizeof("resnet") + 1);
    run_options.size = sizeof("resnet") + 1;
    strcpy(run_options.data, "resnet");

    int64_t dims[] = { 1, 224, 224, 3 };

    struct vaccel_torch_tensor *in;
    struct vaccel_torch_tensor *out;

    ret = vaccel_torch_saved_model_set_path (&model, model_path);

    if (ret) {
        vaccel_debug("Could not set model path to Torch model");
        exit(1);
    }

    vaccel_debug("Created new model %lld\n", vaccel_torch_saved_model_id (&model));

    ret = vaccel_torch_saved_model_register(&model);
    if (ret != 0) {
        return grpc::Status(grpc::StatusCode::INTERNAL,
                            "Failed to register model");
    }

    vaccel_debug("Registered model %lld\n", vaccel_torch_saved_model_id (&model));

    ret = vaccel_sess_register (sess_ptr, model.resource);
    if (ret) {
        fprintf (stderr, "Could not register model with session\n");
        exit (1);
    }

    vaccel_debug("Registered model ID with session pointer");


    in = vaccel_torch_tensor_new (4, dims, VACCEL_TORCH_FLOAT);
    if (!in) {
        fprintf (stderr, "Could not allocate memory\n");
        exit (1);
    }

    vaccel_debug("Created input tensors");



    std::string image_path = "/home/jl/vaccel-torch-cv-example/pic/shark.jpg";
    cv::Mat image = cv::imread(image_path);
    if (image.empty()) {
        fprintf(stderr, "Could not load image %s\n", image_path.c_str());
        exit(1);
    }

    cv::resize(image, image, cv::Size(224, 224));
    image.convertTo(image, CV_32FC3, 1.0 / 255);

    // Manually normalize the image
    std::vector<cv::Mat> channels(3);
    cv::split(image, channels);
    channels[0] = (channels[0] - 0.485) / 0.229;
    channels[1] = (channels[1] - 0.456) / 0.224;
    channels[2] = (channels[2] - 0.406) / 0.225;
    cv::merge(channels, image);

    in = vaccel_torch_tensor_new(4, dims, VACCEL_TORCH_FLOAT);
    if (!in) {
        fprintf(stderr, "Could not allocate memory\n");
        exit(1);
    }

    // std::memcpy(in->data, input_tensor.data_ptr(), input_tensor.numel() * sizeof(float));

    out = (struct vaccel_torch_tensor*) malloc(sizeof(struct vaccel_torch_tensor) * 1);

    int iters = 1; 
    std::vector<int64_t> inference_times_ms;

    for (int i = 0; i < iters; i++) {
        ret = vaccel_torch_jitload_forward(sess_ptr, &model, &run_options, &in, 1, &out, 1);

        if (ret) {
            fprintf(stderr, "Could not run op: %d\n", ret);
            exit(1);
        }
    }

    std::cout << "Success!" << std::endl;
    std::cout << "Result Tensor :" << std::endl;
    std::cout << "Output tensor => type:" << out->data_type << " nr_dims:" << out->nr_dims << std::endl;

    // OutputData outf;
    // outf.data = reinterpret_cast<float*>(out->data);
    // outf.size = 1000;

    // std::vector<size_t> indices(outf.size);
    // for (size_t i = 0; i < outf.size; ++i)
    //     indices[i] = i;

    // std::sort(indices.begin(), indices.end(), [&outf](size_t a, size_t b) {
    //     return outf.data[a] > outf.data[b];
    // });

    // std::vector<float> softmaxed_data = softmax(outf.data, outf.size);

    // for (int i = 0; i < kTOP_K; ++i) {
    //     size_t idx = indices[i];
    //     std::cout << "    ============= Top-" << i + 1 << " =============" << std::endl;
    //     std::cout << "    Label:  " << labels[idx] << std::endl;
    //     std::cout << "    With Probability:  " << softmaxed_data[idx] * 100.0f << "%" << std::endl;
    // }

    ret = vaccel_sess_unregister(sess_ptr, model.resource);
    if (ret) {
        fprintf(stderr, "Could not unregister model with session\n");
    }


    vaccel_debug("Start");

    // ret = vaccel_torch_jitload_forward(sess_ptr, &model, &run_options, &in, 1,
    //                                    &out, 1);
    if (ret != 0) {
        return grpc::Status(grpc::StatusCode::INTERNAL,
                            "Failed to run vaccel_torch_jitload_forward");
    }

    vaccel_debug("Completed");


    return grpc::Status::OK;
}