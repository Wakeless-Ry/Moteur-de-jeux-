#ifndef SCENE_EXAMPLES
#define SCENE_EXAMPLES

#include "FileLoader.cpp"
#include "Scene.h"
#include "src/SceneObject.h"
#include "src/Transform.h"
#include <cstddef>

Scene getPBRbenchmarkScene(size_t nbcarre) {
    Scene scene_result;
    for (size_t i = 0; i < nbcarre; i++) {
        for (size_t j = 0; j < nbcarre; j++) {

            std::optional<Mesh> sphereMesh =
                FileLoader::buildMeshFromOBJ("assets/meshes/sphere.obj");
            if (sphereMesh.has_value()) {
                SceneObject sphere("shaders/PBR_sphere_vs.glsl",
                                   "shaders/PBR_sphere_fs.glsl",
                                   sphereMesh.value());

                sphere.setAlbedo({1., 0., 0.});
                sphere.setMetallic((float)j / (nbcarre - 1));
                sphere.setRoughness((float)i / (nbcarre - 1) + 0.01);

                NodeId id = scene_result.addMesh(sphere);
                scene_result.setTransform(
                    id, translate(i * 0.22, j * 0.22, 0).scale(0.1));
            }
        }
    }
    float intensity = 500.0f;

    scene_result.addLightToScene(
        Light(glm::vec3(-5.0f, 0.0f, 0.0f), glm::vec3(intensity)));

    scene_result.addLightToScene(
        Light(glm::vec3(5.0f, 0.0f, 0.0f), glm::vec3(intensity)));

    scene_result.addLightToScene(
        Light(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(intensity)));

    scene_result.addLightToScene(
        Light(glm::vec3(0.0f, 0.0f, -5.0f), glm::vec3(intensity)));

    return scene_result;
}

Scene getRustedSphereScene() {
    Scene scene_result;

    std::optional<Mesh> sphereMesh =
        FileLoader::buildMeshFromOBJ("assets/meshes/sphere.obj");
    if (sphereMesh.has_value()) {
        SceneObject sphere("shaders/PBR_sphere_vs.glsl",
                           "shaders/PBR_sphere_fs_texture.glsl",
                           sphereMesh.value());

        sphere.addTexture("assets/textures/rustediron2_albedo.png",
                          "albedoMap");

        sphere.addTexture("assets/textures/rustediron2_normal.png",
                          "normalMap");

        sphere.addTexture("assets/textures/rustediron2_metallic.png",
                          "metallicMap");

        sphere.addTexture("assets/textures/rustediron2_roughness.png",
                          "roughnessMap");

        NodeId id = scene_result.addMesh(sphere);
        scene_result.setTransform(id, translate(0, 0, 0).scale(0.1));

        float intensity = 500.0f;

        scene_result.addLightToScene(
            Light(glm::vec3(-5.0f, 0.0f, 0.0f), glm::vec3(intensity)));

        scene_result.addLightToScene(
            Light(glm::vec3(5.0f, 0.0f, 0.0f), glm::vec3(intensity)));

        scene_result.addLightToScene(
            Light(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(intensity)));

        scene_result.addLightToScene(
            Light(glm::vec3(0.0f, 0.0f, -5.0f), glm::vec3(intensity)));
    }

    return scene_result;
}

#endif