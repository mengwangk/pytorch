#pragma once

#include <torch/nn/module.h>
#include <torch/nn/pimpl.h>
#include <torch/optim/optimizer.h>

#include <ATen/ATen.h>

#include <cereal/access.hpp>
#include <cereal/cereal.hpp>

#include <utility>
#include <vector>

namespace torch {
namespace optim {

struct AdamOptions {
  /* implicit */ AdamOptions(double learning_rate);
  TORCH_ARG(double, learning_rate);
  TORCH_ARG(double, beta1) = 0.9;
  TORCH_ARG(double, beta2) = 0.999;
  TORCH_ARG(double, weight_decay) = 0;
  TORCH_ARG(double, eps) = 1e-8;
  TORCH_ARG(bool, amsgrad) = false;
};

class Adam : public Optimizer {
 public:
  template <typename ParameterContainer>
  explicit Adam(ParameterContainer&& parameters, const AdamOptions& options)
      : Optimizer(std::forward<ParameterContainer>(parameters)),
        options_(options),
        step_buffers_(parameters_.size(), 0),
        exp_average_buffers_(zero_buffers_like(parameters_)),
        exp_average_sq_buffers_(zero_buffers_like(parameters_)) {
    if (options_.amsgrad_) {
      max_exp_average_sq_buffers_ = zero_buffers_like(parameters_);
    }
  }

  void step() override;

  template <class Archive>
  void serialize(Archive& ar) {
    ar(CEREAL_NVP(step_buffers_),
       CEREAL_NVP(exp_average_buffers_),
       CEREAL_NVP(exp_average_sq_buffers_),
       CEREAL_NVP(max_exp_average_sq_buffers_));
  }

  const AdamOptions& options() const noexcept;

 private:
  friend class cereal::access;
  Adam() : options_(0) {}

  AdamOptions options_;

  std::vector<int64_t> step_buffers_;
  std::vector<Tensor> exp_average_buffers_;
  std::vector<Tensor> exp_average_sq_buffers_;
  std::vector<Tensor> max_exp_average_sq_buffers_;
};

} // namespace optim
} // namespace torch
