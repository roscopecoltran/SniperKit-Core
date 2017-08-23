#include "BackgroundModel.h"
#include "GLProgram.h"

namespace yarrar
{

BackgroundModel::BackgroundModel(std::weak_ptr<GLProgram> program)
    : m_vao(0)
    , m_vbo(0)
    , m_texture(0)
    , m_program(program)
    , m_numVertices(6)
{
// GL3 uses vertex arrays, but they are not available in
// GLES2.
#ifdef YARRAR_OPENGL_CONTEXT
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);
#endif

    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    GLfloat vertexData[] = {
        // x,y,z, tex_x, tex_y
        -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f,

        1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
    };

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);

    if(auto prog = m_program.lock())
    {
        GLint vertAttrib = prog->getAttrib("vert");
        glEnableVertexAttribArray(vertAttrib);
        glVertexAttribPointer(vertAttrib, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), nullptr);

        GLint texCoordAttrib = prog->getAttrib("texCoord");
        glEnableVertexAttribArray(texCoordAttrib);
        glVertexAttribPointer(texCoordAttrib, 2, GL_FLOAT, GL_TRUE, 5 * sizeof(GLfloat),
            (const GLvoid*) (3 * sizeof(GLfloat)));
    }
    else
    {
        throw std::runtime_error("Program not available.");
    }


    glBindBuffer(GL_ARRAY_BUFFER, 0);
#ifdef YARRAR_OPENGL_CONTEXT
    glBindVertexArray(0);
#endif
}

void BackgroundModel::render() const
{
    if(auto prog = m_program.lock())
    {
        ScopedUseProgram _p(prog.get());
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_texture);

        glUniform1i(prog->getUniform("tex"), 0);
#ifdef YARRAR_OPENGL_CONTEXT
        glBindVertexArray(m_vao);
#elif YARRAR_OPENGLES_CONTEXT
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        GLint vertAttrib = prog->getAttrib("vert");
        glEnableVertexAttribArray(vertAttrib);
        glVertexAttribPointer(vertAttrib, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), nullptr);

        GLint texCoordAttrib = prog->getAttrib("texCoord");
        glEnableVertexAttribArray(texCoordAttrib);
        glVertexAttribPointer(texCoordAttrib, 2, GL_FLOAT, GL_TRUE, 5 * sizeof(GLfloat),
            (const GLvoid*) (3 * sizeof(GLfloat)));
#endif

        glDrawArrays(GL_TRIANGLES, 0, m_numVertices);

#ifdef YARRAR_OPENGL_CONTEXT
        glBindVertexArray(0);
#elif YARRAR_OPENGLES_CONTEXT
        glDisableVertexAttribArray(vertAttrib);
        glDisableVertexAttribArray(texCoordAttrib);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
#endif
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    else
    {
        throw std::runtime_error("Program not available.");
    }
}

void BackgroundModel::setTexture(GLuint texture)
{
    m_texture = texture;
}
}
