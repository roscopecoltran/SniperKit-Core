#pragma once

#include "GLContext.h"
#include "Pipeline.h"
#include "Types.h"

#include <string>
#include <map>

namespace yarrar
{

class GLShader;
class GLProgram;
class BackgroundModel;
class SceneModel;

struct EffectDef
{
    std::string name;
    std::string vertexShader;
    std::string fragmentShader;
    bool perspectiveProjection;
};

struct ShaderDef
{
    std::string name;
    std::string path;
};

class OpenGLRenderer : public Renderer
{
public:
    OpenGLRenderer(int width, int height, const json11::Json& config);
    ~OpenGLRenderer();
    OpenGLRenderer(const OpenGLRenderer&) = delete;
    OpenGLRenderer operator=(const OpenGLRenderer&) = delete;
    OpenGLRenderer(const OpenGLRenderer&&) = delete;
    OpenGLRenderer operator=(const OpenGLRenderer&&) = delete;

    void loadModel(const Model& model) override;

    void draw(const std::vector<Pose>& cameraPoses,
        const Scene& scene,
        const Datapoint& rawData) override;

private:
    void drawCoordinateSystem(const Pose& cameraPose,
        const Scene& scene);
    void render(int coordinateSystemId, const Scene& scene);
    void loadImage(const cv::Mat& image);

    std::map<std::string, std::unique_ptr<GLShader>> m_vertexShaders;
    std::map<std::string, std::unique_ptr<GLShader>> m_fragmentShaders;
    std::map<std::string, std::shared_ptr<GLProgram>> m_programs;

    std::unique_ptr<GLContext> m_context;
    std::unique_ptr<BackgroundModel> m_bgModel;
    std::map<std::string, std::unique_ptr<SceneModel>> m_sceneModels;
    GLuint m_backgroundTex;
    Timestamp m_lastUpdatedBackground;
};
}
