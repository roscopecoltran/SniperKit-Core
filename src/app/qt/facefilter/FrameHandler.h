/*!
  @file   finder/FrameHandler.h
  @author David Hirvonen
  @brief Common state and stored frame handlers.

  \copyright Copyright 2014-2016 Elucideye, Inc. All rights reserved.
  \license{This project is released under the 3 Clause BSD License.}

*/

#ifndef __drishti_qt_facefilter_FrameHandler_h__
#define __drishti_qt_facefilter_FrameHandler_h__

#include "drishti/hci/FaceMonitor.h"

#include <opencv2/core/core.hpp>

#include "thread_pool/thread_pool.hpp"

#include "nlohmann_json.hpp" // nlohman-json

#include <functional>
#include <vector>
#include <memory>

// clang-format off
namespace drishti
{
    namespace core {  class ImageLogger; }
    namespace sensor { class SensorModel; };
};
namespace spdlog { class logger; }
// clang-format on

class FrameHandlerManager
{
public:
    struct DetectionParams
    {
        float m_minDepth; // meters
        float m_maxDepth; // meters
        float m_interval; // seconds
    };

    using Settings = nlohmann::json;
    using FrameHandler = std::function<void(const cv::Mat&)>;

    FrameHandlerManager(Settings* settings, const std::string& name, const std::string& description);

    ~FrameHandlerManager();

    bool good() const;

    static FrameHandlerManager* get(Settings* settings = nullptr, const std::string& name = {}, const std::string& description = {});

    int getOrientation() const
    {
        return m_orientation;
    }

    void setOrientation(int orientation)
    {
        m_orientation = orientation;
    }

    void setDeviceInfo(const std::string& name, const std::string& description)
    {
        m_deviceName = name;
        m_deviceDescription = description;
    }

    void setSize(const cv::Size& size);

    cv::Size getSize() const;

    void add(FrameHandler& handler)
    {
        m_handlers.push_back(handler);
    }

    std::vector<FrameHandler>& getHandlers()
    {
        return m_handlers;
    }

    std::shared_ptr<drishti::sensor::SensorModel>& getSensor()
    {
        return m_sensor;
    }

    std::shared_ptr<spdlog::logger>& getLogger()
    {
        return m_logger;
    }

    std::shared_ptr<tp::ThreadPool<>>& getThreadPool()
    {
        return m_threads;
    }

    FrameHandler createAsynchronousImageLogger();
#if DRISHTI_USE_BEAST
    std::shared_ptr<drishti::core::ImageLogger>& getImageLogger()
    {
        return m_imageLogger;
    }
#endif

    const DetectionParams& getDetectionParameters()
    {
        return m_detectionParams;
    }

    drishti::hci::FaceMonitor* getFaceMonitor()
    {
        return m_faceMonitor.get();
    }

    Settings* getSettings() { return m_settings; }
    const Settings* getSettings() const { return m_settings; }

protected:
    Settings* m_settings = nullptr;
    DetectionParams m_detectionParams;
    int m_orientation = 0;
    std::string m_deviceName;
    std::string m_deviceDescription;
    std::shared_ptr<spdlog::logger> m_logger;
    std::shared_ptr<tp::ThreadPool<>> m_threads;
    std::shared_ptr<drishti::sensor::SensorModel> m_sensor;
    std::vector<FrameHandler> m_handlers;
    std::unique_ptr<drishti::hci::FaceMonitor> m_faceMonitor;

#if DRISHTI_USE_BEAST
    std::shared_ptr<drishti::core::ImageLogger> m_imageLogger;
#endif

    static FrameHandlerManager* m_instance;
};

#endif // __drishti_qt_facefilter_FrameHandler_h__
