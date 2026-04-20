#ifndef PVS_
#define PVS_

#include "src/GlobalScene.h"
#include <optional>
#include <set>

class PVS {
    GlobalScene &globalScene;

    std::set<NodeId> scenes;

    std::optional<NodeId> currentScene;

    std::map<NodeId, std::set<NodeId>> graph;

    void hideConnected(NodeId scene) {
        this->globalScene.hideNode(scene);

        for (NodeId connected : graph[scene]) {
            this->globalScene.hideNode(connected);
        }
    }

    void showConnected(NodeId scene) {
        this->globalScene.showNode(scene);

        for (NodeId connected : graph[scene]) {
            this->globalScene.showNode(connected);
        }
    }

  public:
    PVS(GlobalScene &globalScene) : globalScene(globalScene) {}

    void enterScene(NodeId scene) {
        if (this->currentScene.has_value()) {
            this->hideConnected(this->currentScene.value());
        }

        this->currentScene = scene;

        this->showConnected(scene);
    }

    void exitScene(NodeId scene) {
        if (this->currentScene.has_value()) {
            this->hideConnected(this->currentScene.value());
        }

        this->currentScene = std::nullopt;
    }

    NodeId addScene() {
        NodeId newScene = this->globalScene.addBasicNode();

        this->scenes.insert(newScene);

        return newScene;
    }

    void removeScene(NodeId scene) {
        this->globalScene.removeNode(scene);
        this->scenes.erase(scene);
    }

    void linkScenes(NodeId first, NodeId second) {
        this->graph[first].insert(second);
        this->graph[second].insert(first);
    }

    std::optional<NodeId> getCurrentScene() { return currentScene; }
};

#endif // PVS_