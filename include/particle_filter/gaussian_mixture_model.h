//#########################################################//
//#                                                       #//
//# gaussian_mixture_models  gaussian_mixture_model.h     #//
//# Roberto Capobianco  <capobianco@dis.uniroma1.it>      #//
//#                                                       #//
//#########################################################//

#ifndef _GAUSSIAN_MIXTURE_MODEL_H_
#define _GAUSSIAN_MIXTURE_MODEL_H_

#include <algorithm>
#include <Eigen/Core>
#include <geometry_msgs/Point.h>
#include <std_msgs/ColorRGBA.h>
#include <stdexcept>
#include <string>
#include <vector>
#include <visualization_msgs/Marker.h>

#include <particle_filter/gaussian.h>
#include <particle_filter/k_means.h>
#include <particle_filter/matrix_io.h>

namespace gmms {
class GaussianMixtureModel {
public:
    GaussianMixtureModel() {
        num_components_ = 1;
        delta_ = 0.01;
        num_iterations_ = 100;
        initialized_ = false;
    }
    GaussianMixtureModel(const int num_components,
            const double delta = 0.01,
            const int num_iterations = 100) {
        num_components_ = num_components;
        prior_vec_.resize(num_components_);
        gaussian_vec_.resize(num_components_);
        delta_ = delta;
        num_iterations_ = num_iterations;
        initialized_ = false;
    }

    /*
     explicit GaussianMixtureModel(const GaussianMixtureModel& gmm) {
       num_components_ = gmm.numComponents();
       delta_ = gmm.delta();
       num_iterations_ = gmm.numIterations();
       initialized_ = gmm.initialized();
       prior_vec_ = std::vector<double>(gmm.priorVec());
       gaussian_vec_ = std::vector<Gaussian>(gmm.gaussianVec());
       expectations_ = gmm.expectations().replicate(1, 1);
     }
    */

    // dataset with each datapoint per row and each dimension per column
    void initialize(const Eigen::MatrixXd& dataset);
    void expectationMaximization(const Eigen::MatrixXd& dataset);
    double logLikelihood(const Eigen::MatrixXd& dataset);
    double bayesianInformationCriterion(const Eigen::MatrixXd& dataset);
    void load(Eigen::MatrixXd model);
    std::pair<Gaussian, double>
    getClosestGaussian(const Eigen::VectorXd& mean) const;
    GaussianMixtureModel copy() const;
    Eigen::MatrixXd save();

    inline void load(const std::string filename) {
        MatrixIO mio;
        Eigen::MatrixXd model;
        mio.readFromFile(filename, model);
        load(model);
    }

    inline void save(const std::string filename) {
        MatrixIO mio;
        mio.writeToFile(filename, save());
    }

    // setter & getter
    inline int numComponents() const {
        return num_components_;
    }
    inline void setNumComponents(const int num_components) {
        num_components_ = num_components;
        prior_vec_.resize(num_components_);
        gaussian_vec_.resize(num_components_);
    }
    inline double delta() const {
        return delta_;
    }
    inline void setDelta(const double delta) {
        delta_ = delta;
    }
    inline int numIterations() const {
        return num_iterations_;
    }
    inline bool initialized() const {
        return initialized_;
    }
    inline Eigen::MatrixXd expectations() const {
        return expectations_;
    }
    inline std::vector<double> priorVec() const {
        return prior_vec_;
    }
    inline std::vector<Gaussian> gaussianVec() const {
        return gaussian_vec_;
    }
    inline void setNumIterations(const int num_iterations) {
        num_iterations_ = num_iterations;
    }
    inline const Gaussian& component(int i) {
        if (i > num_components_) {
            throw std::runtime_error(notexisting_());
        }

        return gaussian_vec_[i];
    }
    inline const std::vector<Eigen::VectorXd> gaussianMeans() const {
        std::vector<Eigen::VectorXd> means(gaussian_vec_.size());
        for (int g = 0; g < gaussian_vec_.size(); ++g) {
            means.at(g) = gaussian_vec_.at(g).mean();
        }
        return means;
    }
    inline const std::vector<Eigen::MatrixXd> gaussianCovariances() const {
        std::vector<Eigen::MatrixXd> covs(gaussian_vec_.size());
        for (int g = 0; g < gaussian_vec_.size(); ++g) {
            covs.at(g) = gaussian_vec_.at(g).covariance();
        }
        return covs;
    }
    std::string toString() const;

    visualization_msgs::Marker renderMarker(float x0,
            float y0,
            float x1,
            float y1,
            int stepcount,
            std::string n_space,
            std::string frame,
            ros::Duration lifetime,
            bool use_color = true,
            bool use_height = true) const;

private:
    bool initialized_;
    double delta_;
    int num_iterations_;
    int num_components_;
    Eigen::MatrixXd expectations_;
    std::vector<double> prior_vec_;
    std::vector<Gaussian> gaussian_vec_;

    GaussianMixtureModel(int num_components,
            double delta,
            int num_iterations,
            std::vector<double> prior_vec_,
            std::vector<Gaussian> gaussian_vec_,
            Eigen::MatrixXd expectations_) {
        num_components_ = num_components;
        prior_vec_ = std::vector<double>(prior_vec_);
        gaussian_vec_ = std::vector<Gaussian>(gaussian_vec_);
        expectations_ = expectations_.replicate(1, 1);
        delta_ = delta;
        num_iterations_ = num_iterations;
        initialized_ = false;
    }

    inline double probability_density_function(const Eigen::VectorXd point) {
        double probability = 0;

        for (int k = 0; k < num_components_; ++k) {
            probability +=
                    prior_vec_[k] * gaussian_vec_[k].evaluate_point(point);
        }

        return probability;
    }

    void expectation(const Eigen::MatrixXd& dataset);
    void maximization(const Eigen::MatrixXd& dataset);

    inline static const std::string zero_components() {
        return "The number of components of the GMM has not been specified";
    }

    inline static const std::string notinitialized_() {
        return "The GMM has not been initialized";
    }

    inline static const std::string notexisting_() {
        return "The desired component does not exist";
    }
};
}  // namespace gmms

#endif  // _GAUSSIAN_MIXTURE_MODEL_H_
