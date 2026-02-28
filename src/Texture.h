#ifndef TEXTURE
#define TEXTURE

#include <GL/glew.h>
#include <GLFW/glfw3.h>

using uint = unsigned int;

class Texture {
    uint texId;
    int pos;

  public:
    Texture();
    Texture(const char *path, int pos);

    uint getId() const;
    void bind(GLuint programId, const char *name) const;
    void cleanUp();
};

#endif // TEXTURE