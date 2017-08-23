#ifndef __drishti_qt_facefilter_QtFaceDetectorFactory_h__
#define __drishti_qt_facefilter_QtFaceDetectorFactory_h__

#include "drishti/face/FaceDetectorFactory.h"

#include <functional> // std::function

// clang-format off
namespace drishti
{
    namespace ml
    {
        class ObjectDetector;
        class ShapeEstimator;
    }
}
// clang-format on

class QtFaceDetectorFactory : public drishti::face::FaceDetectorFactory
{
public:
    using LoaderFunction = std::function<bool(std::istream& is, const std::string& hint)>;

    QtFaceDetectorFactory();

    virtual std::unique_ptr<drishti::ml::ObjectDetector> getFaceDetector();
    virtual std::unique_ptr<drishti::ml::ShapeEstimator> getFaceEstimator();
    virtual std::unique_ptr<drishti::eye::EyeModelEstimator> getEyeEstimator();
    virtual drishti::face::FaceModel getMeanFace();

    static bool load(const std::string& filename, LoaderFunction& loader);
};

#endif // __drishti_qt_facefilter_QtFaceDetectorFactory_h__
