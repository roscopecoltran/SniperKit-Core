/*!
  @file   test-RTEShapeEstimator.cpp
  @author David Hirvonen
  @brief  Google fixture with various subtests for the RegressionTreeEnsembleShapeEstimator

  \copyright Copyright 2014-2016 Elucideye, Inc. All rights reserved.
  \license{This project is released under the 3 Clause BSD License.}

*/

#include <gtest/gtest.h>

// These must come before drishti_cv.hpp
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include "drishti/core/drishti_stdlib_string.h"
#include "drishti/core/drishti_cereal_pba.h"
#include "drishti/core/drishti_cv_cereal.h"
#include "drishti/ml/RegressionTreeEnsembleShapeEstimator.h"

#include <fstream>

// https://code.google.com/p/googletest/wiki/Primer

const char* modelFilename;
const char* imageFilename;
const char* truthFilename;
const char* outputDirectory;
bool isTextArchive;

// clang-format off
#define BEGIN_EMPTY_NAMESPACE  namespace {
#define END_EMPTY_NAMESPACE }
// clang-format on

BEGIN_EMPTY_NAMESPACE

class RTEShapeEstimatorTest : public ::testing::Test
{
public:
    static std::shared_ptr<drishti::ml::RegressionTreeEnsembleShapeEstimator> create(const std::string& filename)
    {
        return std::make_shared<drishti::ml::RegressionTreeEnsembleShapeEstimator>(filename);
    }

protected:
    // Setup
    RTEShapeEstimatorTest()
    {
        // Create the segmenter (constructor tests performed prior to this)
        m_shapePredictor = create(modelFilename);

        // Load the ground truth data:
        loadTruth();

        // Load sample for each image width:
        loadImages();
    }

    // Cleanup
    virtual ~RTEShapeEstimatorTest() {}

    // Called after constructor for each test
    virtual void SetUp() {}

    // Called after destructor for each test
    virtual void TearDown() {}

    // Utility methods:
    void loadImages()
    {
        assert(imageFilename);

        // First get our 4x3 aspect ratio image
        cv::Mat image = cv::imread(imageFilename, cv::IMREAD_GRAYSCALE);
        assert(!image.empty());

        m_image = image;
    }

    void loadTruth()
    {
        assert(truthFilename);
        std::ifstream is(truthFilename);
        if (is)
        {
            // TODO: Load ground truth model here
        }
        else
        {
            std::cerr << "Unable to find ground truth file: " << truthFilename << std::endl;
        }
    }

    // Objects declared here can be used by all tests in the test case for RTEShapeEstimator.
    std::shared_ptr<drishti::ml::RegressionTreeEnsembleShapeEstimator> m_shapePredictor;

    int m_targetWidth = 127;

    // Ground truth image:
    cv::Mat m_image;

    // Score tolerance:
    float m_scoreThreshold = 0.60;
};

/*
 * Basic class construction
 */

TEST_F(RTEShapeEstimatorTest, CerealSerialization)
{
    { // Enable this block to dump regressor leaf nodes (offline analysis):
      // static const bool pca = true;
      // std::vector<float> values;
      // m_shapePredictor->dump(values, pca);
      //
      // std::ofstream ofs("/tmp/points_pca.cpb", std::ios_base::binary);
      // if(ofs)
      // {
      //     cereal::PortableBinaryOutputArchive oa(ofs);
      //     oa << values;
      // }
    }

    std::string filename = std::string(outputDirectory) + "/shape.cpb";
    save_cpb(filename, *m_shapePredictor);
    drishti::ml::RegressionTreeEnsembleShapeEstimator shapePredictor2;
    load_cpb(filename, shapePredictor2);
}

/*
 * Fixture tests
 */

TEST_F(RTEShapeEstimatorTest, ShapeSerialization)
{
    std::vector<bool> mask;
    std::vector<cv::Point2f> points;
    /* int code = */ (*m_shapePredictor)(m_image, points, mask);
}

END_EMPTY_NAMESPACE
