#pragma once

#include <algorithm>
#include <cmath>
#include <vector>

#include <GL/glew.h>
#include <glm/ext.hpp>

#include "src/AssetManager.h"
#include "src/Camera.h"
#include "src/Light.hpp"
#include "src/SceneObject.h"
#include "src/Transform.h"
#include "src/ecs/ECSManager.h"
#include "src/ecs/components/Positionable.h"
#include "src/ecs/components/VerletBody.h"

class VelocityIndicatorHud {
  public:
    const char *sphereMeshPath = "assets/meshes/big_sphere.obj";
    const char *arrowMeshPath = "assets/meshes/arrow.obj";

    const char *vsPath = "shaders/PBR_vs.glsl";
    const char *fsPath = "shaders/PBR_fs.glsl";

    int hudSizePx = 220;
    int hudMarginPx = 20;
};