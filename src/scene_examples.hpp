#ifndef SCENE_EXAMPLES
#define SCENE_EXAMPLES

#include "FileLoader.hpp"
#include "Scene.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "src/Transform.hpp"
#include <cstddef>

Scene getPBRbenchmarkScene(size_t nbcarre) {
    Scene scene_result;
    for (size_t i = 0; i < nbcarre; i++) {
        for (size_t j = 0; j < nbcarre; j++) {

            std::optional<Mesh> sphereMesh = FileLoader::buildMeshFromOBJ(
                "shaders/PBR_sphere_vs.glsl", "shaders/PBR_sphere_fs.glsl",
                "assets/sphere.obj");
            if (sphereMesh.has_value()) {
                Mesh sphere = sphereMesh.value();

                sphere.setAlbedo({1., 0., 0.});
                sphere.setMetallic((float)j / (nbcarre - 1));
                sphere.setRoughness((float)i / (nbcarre - 1) + 0.01);

                NodeId id = scene_result.addMesh(sphere);
                scene_result.setTransform(
                    id,
                    Transform().translate(i * 0.22, j * 0.22, 0).scale(0.1));
            }
        }
    }
    return scene_result;
}

Scene getPBRbenchmarkSceneTexture(size_t nbcarre) {
    Scene scene_result;
    for (size_t i = 0; i < nbcarre; i++) {
        for (size_t j = 0; j < nbcarre; j++) {

            std::optional<Mesh> sphereMesh = FileLoader::buildMeshFromOBJ(
                "shaders/PBR_sphere_vs.glsl",
                "shaders/PBR_sphere_fs_texture.glsl", "assets/big_sphere2.obj");
            if (sphereMesh.has_value()) {
                Mesh sphere = sphereMesh.value();

                sphere.addTexture("assets/rustediron2_basecolor.png",
                                  "albedoMap");

                sphere.addTexture("assets/rustediron2_normal.png", "normalMap");

                sphere.addTexture("assets/rustediron2_metallic.png",
                                  "metallicMap");

                sphere.addTexture("assets/rustediron2_roughness.png",
                                  "roughnessMap");

                NodeId id = scene_result.addMesh(sphere);
                scene_result.setTransform(
                    id,
                    Transform().translate(i * 0.22, j * 0.22, 0).scale(0.1));
            }
        }
    }
    return scene_result;
}

#endif