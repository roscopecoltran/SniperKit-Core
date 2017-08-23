#include "QtFaceDetectorFactory.h"

#include <QDirIterator>
#include <QFile>
#include <QTextStream>
#include <QDirIterator>
#include "QtStream.h"

#include "drishti/core/drishti_core.h"
#include "drishti/core/drishti_string_hash.h"
#include "drishti/core/make_unique.h"
#include "drishti/acf/ACF.h"
#include "drishti/ml/RegressionTreeEnsembleShapeEstimator.h"
#include "drishti/eye/EyeModelEstimator.h"

#include "drishti/core/drishti_cereal_pba.h"

#include "nlohmann_json.hpp" // nlohman-json + ANDROID stdlib patch

#include <iostream>

using namespace string_hash;

bool QtFaceDetectorFactory::load(const std::string& filename, LoaderFunction& loader)
{
    std::string resource;

    QDirIterator it(":", QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        std::string name = it.next().toStdString();
        auto pos = name.find(filename);
        if (pos != std::string::npos)
        {
            resource = name;
            break;
        }
    }

    if (resource.empty())
    {
        return false;
    }

    // Then open it:
    QFile inputFile(resource.c_str());
    if (!inputFile.open(QIODevice::ReadOnly))
    {
        std::cerr << "Can't open file" << std::endl;
        return false;
    }

    QByteArray raw(inputFile.readAll());
    QtStream streambuf(raw);
    std::istream is(&streambuf);
    return loader(is, filename);
}

QtFaceDetectorFactory::QtFaceDetectorFactory()
{
    // clang-format off
    LoaderFunction loader = [&](std::istream& is, const std::string& hint)
    {
        if(is)
        {
            nlohmann::json object;
            is >> object;
            sEyeRegressor = object["eye_model_regressor"].get<std::string>();
            sFaceRegressor = object["face_landmark_regressor"].get<std::string>();
            sFaceDetector = object["face_detector"].get<std::string>();
            sFaceDetectorMean = object["face_detector_mean"].get<std::string>();
            return true;
        }
        return false;
    };
    // clang-format on
    
    load("drishti_assets.json", loader);
}

std::unique_ptr<drishti::ml::ObjectDetector> QtFaceDetectorFactory::getFaceDetector()
{
    std::unique_ptr<drishti::ml::ObjectDetector> ptr;

    // clang-format off
    LoaderFunction loader = [&](std::istream& is, const std::string& hint)
    {
        ptr = drishti::core::make_unique<drishti::acf::Detector>(is, hint);
        return true;
    };
    // clang-format on

    load(sFaceDetector, loader);

    return ptr;
}

std::unique_ptr<drishti::ml::ShapeEstimator> QtFaceDetectorFactory::getFaceEstimator()
{
    std::unique_ptr<drishti::ml::ShapeEstimator> ptr;

    // clang-format off
    LoaderFunction loader = [&](std::istream& is, const std::string& hint)
    {
        ptr = drishti::core::make_unique<drishti::ml::RegressionTreeEnsembleShapeEstimator>(is, hint);
        return true;
    };
    // clang-format on
    if (!sFaceRegressor.empty())
    {
        load(sFaceRegressor, loader);
    }
    return ptr;
}

std::unique_ptr<drishti::eye::EyeModelEstimator> QtFaceDetectorFactory::getEyeEstimator()
{
    std::unique_ptr<drishti::eye::EyeModelEstimator> ptr;
    LoaderFunction loader = [&](std::istream& is, const std::string& hint) {
        ptr = drishti::core::make_unique<DRISHTI_EYE::EyeModelEstimator>(is, hint);
        return true;
    };
    load(sEyeRegressor, loader);
    return ptr;
}
