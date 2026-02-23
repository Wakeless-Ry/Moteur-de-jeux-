#ifndef SCENE_GRAPH
#define SCENE_GRAPH

#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <functional>

#include "Scenenode.hpp"
#include "Camera.hpp"

using namespace std;

class SceneGraph {
private:
    SceneNode* root;
    map<string, SceneNode*> nodeRegistry;
    vector<SceneNode*> allNodes;

public:
    SceneGraph() {
        root = new SceneNode("Root");
        allNodes.push_back(root);
        nodeRegistry["Root"] = root;
    }

    ~SceneGraph() {
        clear();
    }

    SceneNode* getRoot() const {
        return root;
    }

    SceneNode* createNode(const string& name, SceneNode* parent = nullptr) {
        SceneNode* node = new SceneNode(name);
        allNodes.push_back(node);
        
        string uniqueName = name;
        if (nodeRegistry.find(uniqueName) != nodeRegistry.end()) {
            int counter = 1;
            while (nodeRegistry.find(uniqueName) != nodeRegistry.end()) {
                uniqueName = name + "_" + to_string(counter++);
            }
            node->setName(uniqueName);
        }
        nodeRegistry[node->getName()] = node;
        
        // Set parent
        if (parent) {
            parent->addChild(node);
        } else {
            root->addChild(node);
        }
        
        return node;
    }

    void addNode(SceneNode* node, SceneNode* parent = nullptr) {
        if (!node) return;
        
        if (find(allNodes.begin(), allNodes.end(), node) != allNodes.end()) {
            return;
        }
        
        allNodes.push_back(node);
        
        string name = node->getName();
        string uniqueName = name;
        if (nodeRegistry.find(uniqueName) != nodeRegistry.end()) {
            int counter = 1;
            while (nodeRegistry.find(uniqueName) != nodeRegistry.end()) {
                uniqueName = name + "_" + to_string(counter++);
            }
            node->setName(uniqueName);
        }
        nodeRegistry[node->getName()] = node;
        
        if (parent) {
            parent->addChild(node);
        } else {
            root->addChild(node);
        }
    }

    void removeNode(SceneNode* node) {
        if (!node || node == root) return;
        
        // remove children
        vector<SceneNode*> childrenCopy = node->getChildren();
        for (auto child : childrenCopy) {
            removeNode(child);
        }
        
        // Remove from parent
        if (node->getParent()) {
            node->getParent()->removeChild(node);
        }
        
        // Remove from registry
        nodeRegistry.erase(node->getName());
        
        // Remove from allNodes and delete
        auto it = find(allNodes.begin(), allNodes.end(), node);
        if (it != allNodes.end()) {
            allNodes.erase(it);
            delete node;
        }
    }

    SceneNode* findNode(const string& name) const {
        auto it = nodeRegistry.find(name);
        if (it != nodeRegistry.end()) {
            return it->second;
        }
        return nullptr;
    }

    // Update travel
    void update(float deltaTime) {
        if (root) {
            glm::mat4 identityMatrix = glm::mat4(1.0f);
            root->update(deltaTime, identityMatrix);
        }
    }

    // Render travel
    void render(const Camera& camera) {
        if (root) {
            root->render(camera);
        }
    }

    void clear() {
        for (auto node : allNodes) {
            if (node != root) {
                delete node;
            }
        }
        
        if (root) {
            vector<SceneNode*> children = root->getChildren();
            for (auto child : children) {
                root->removeChild(child);
            }
        }
        
        allNodes.clear();
        nodeRegistry.clear();
        
        if (root) {
            delete root;
        }
        root = new SceneNode("Root");
        allNodes.push_back(root);
        nodeRegistry["Root"] = root;
    }

    size_t getNodeCount() const {
        return allNodes.size();
    }

    void traverse(function<void(SceneNode*)> visitor) {
        if (root) {
            traverseRecursive(root, visitor);
        }
    }

private:
    void traverseRecursive(SceneNode* node, function<void(SceneNode*)>& visitor) {
        if (!node) return;
        
        visitor(node);
        
        for (auto child : node->getChildren()) {
            traverseRecursive(child, visitor);
        }
    }
};

#endif // SCENE_GRAPH