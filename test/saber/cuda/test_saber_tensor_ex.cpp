
#include "core/context.h"
#include "test_saber_func_NV.h"
#include "tensor_op.h"
#include "saber_types.h"
#include <vector>

using namespace anakin::saber;
template<typename TensorType>
void test_argmax(std::vector<TensorType*>& inputs_big, std::vector<TensorType*>& outputs_big,
                 std::vector<TensorType*>& inputs, std::vector<TensorType*>& outputs,
                 Shape input_offset, Shape output_offset,
                 bool input_share, bool output_share, bool input_share_sub, bool output_share_sub,
                 bool get_time, Context<NV>& ctx) {
    typedef Tensor<NV, AK_FLOAT, NCHW> TensorDf4;
    typedef Tensor<X86, AK_FLOAT, NCHW> TensorHf4;
    /*prepare data*/

    Shape output_shape{inputs[0]->num(), 1, 1, 1};

    outputs[0]->set_shape(output_shape);
    inputs[0]->set_shape(inputs[0]->valid_shape(), inputs[0]->valid_shape());

    if (output_share && output_share_sub) {
        outputs[0]->share_sub_buffer(*outputs_big[0], outputs[0]->valid_shape(), output_offset);
    } else if (output_share) {
        outputs[0]->share_from(*outputs_big[0]);
    } else {
        outputs[0]->re_alloc(outputs[0]->valid_shape());
    }

    if (input_share && input_share_sub) {
        inputs[0]->share_sub_buffer(*inputs_big[0], inputs[0]->valid_shape(), input_offset);
    } else if (input_share) {
        inputs[0]->share_from(*inputs_big[0]);
    } else {
        //inputs[0]->re_alloc(inputs[0]->valid_shape());
        //inputs[0]->reshape(Shape{1, 100, 100, 100});
        LOG(INFO) << "device inputs valid size = " << inputs[0]->valid_size();
        inputs[0]->reshape(Shape{1, 1, 10, 100});
        LOG(INFO) << "device inputs big, valid size = " << inputs[0]->valid_size();
        //print_tensor_device(*inputs_big[0]);
        //cudaDeviceSynchronize();
        Shape shape_in = inputs[0]->valid_shape();
        LOG(INFO) << "inputs valid size: " << shape_in[0] << ", " << shape_in[1] << ", " << shape_in[2] <<
                  ", " << shape_in[3];


#if 0
        //cudaMemcpyAsync(inputs[0]->mutable_data(), inputs_big[0]->data(),\
        sizeof(float)*(inputs[0]->valid_size()), cudaMemcpyDeviceToDevice, ctx.get_compute_stream());
        //cudaMemset(inputs[0]->mutable_data(), 0, inputs[0]->valid_size() * sizeof(float));
        //CUDA_CHECK(cudaMemcpy(inputs[0]->mutable_data(), inputs_big[0]->data(),\
        inputs[0]->valid_size() * sizeof(float), cudaMemcpyDeviceToDevice));
        //fill_tensor_device_const(*inputs[0], 1.f);
        inputs_big[0]->set_shape(shape_in);
        inputs[0]->copy_from(*inputs_big[0]);
        LOG(INFO) << "device inputs, vailid size = " << inputs[0]->valid_size();
        //print_tensor_device(*inputs[0]);
        //cudaDeviceSynchronize();
        CUDA_CHECK(cudaPeekAtLastError());
        //TensorHf4 th(inputs[0]->valid_shape());
        //th.copy_from(*inputs[0]);
        //print_tensor_host(th);
#endif

    }
}

TEST(TestSaberFuncNV, test_func_argmax) {
    Env<NV>::env_init();
    typedef TargetWrapper<X86> X86_API;
    typedef TargetWrapper<NV> NV_API;
    typedef Tensor<X86, AK_FLOAT, NCHW> TensorHf4;
    typedef Tensor<NV, AK_FLOAT, NCHW> TensorDf4;

    int n = 1;
    int c = 3;
    int h = 128;
    int w = 4;

    //Shape img_s(img_num, in_channels, img_h, img_w);
    Shape real_shape(n + 1, c + 1, h + 1, w + 1);
    Shape valid_shape(n, c, h, w);
    Shape input_offset(0, 0, 0, 0);
    Shape output_offset(0, 0, 0, 0);

    TensorHf4 in_host_big;
    TensorDf4 in_dev_big;
    TensorHf4 out_host_big;
    TensorDf4 out_dev_big;

    if (1) {
        TensorDf4 td0;
        td0.set_shape(Shape{100, 100, 100, 100});

        if (1) {
            TensorDf4 td1(Shape{100, 100, 100, 100});
            td0.share_from(td1);
            TensorDf4 td2(Shape{100, 100, 100, 100});
            sleep(5);
        }

        LOG(INFO) << "shared mem size " << td0.size();

        sleep(5);
    }

    sleep(5);

    TensorDf4 in_dev;
    TensorDf4 output_dev;
    //in_dev.re_alloc(valid_shape);


    in_host_big.re_alloc(real_shape);
    in_dev_big.re_alloc(real_shape);
    out_host_big.re_alloc(real_shape);
    out_dev_big.re_alloc(real_shape);

    /*prepare input data*/
    auto data = in_host_big.mutable_data();

    for (int i = 0; i < in_host_big.size(); ++i) {
        //data[i] = 0x7f &i;
        data[i] = i;
    }

    in_dev_big.share_from(in_host_big);

    in_dev.set_shape(valid_shape);

    std::vector<TensorDf4*> inputs_big;
    std::vector<TensorDf4*> outputs_big;
    std::vector<TensorDf4*> inputs;
    std::vector<TensorDf4*> outputs;
    inputs_big.push_back(&in_dev_big);
    outputs_big.push_back(&out_dev_big);
    inputs.push_back(&in_dev);
    outputs.push_back(&output_dev);

    // start Reshape & doInfer
    Context<NV> ctx(0, 1, 1);
    /*bool out_max_val_in,int top_k_in, int axis_in*/

    for (auto input_share : {
                true, false
            }) {
        for (auto output_share : {
                    false
                }) {
            for (auto input_share_sub : {
                        false
                    }) {
                for (auto output_share_sub : {
                            false
                        }) {
                    for (auto get_time : {
                                false
                            }) {
                        LOG(INFO) << input_share << "," << output_share << "," << input_share_sub << "," << output_share_sub
                                  << "," << get_time;
                        test_argmax<TensorDf4>(inputs_big, outputs_big,
                                               inputs, outputs,
                                               input_offset, output_offset,
                                               input_share, output_share, input_share_sub, output_share_sub, get_time, ctx);
                    }
                }
            }
        }
    }
}

int main(int argc, const char** argv) {
    // initial logger
    //logger::init(argv[0]);
    InitTest();
    RUN_ALL_TESTS(argv[0]);
    return 0;
}

