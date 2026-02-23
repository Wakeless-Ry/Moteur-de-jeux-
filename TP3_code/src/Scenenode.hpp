#ifndef SCENE_NODE
#define SCENE_NODE

#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Transform.hpp"
#include "Mesh.hpp"
#include "Camera.hpp"

using namespace std;

class SceneNode {
protected:
    string name;
    bool active;
    
    Transform transform;
    
    SceneNode* parent;
    vector<SceneNode*> children;
    
    Mesh* mesh;

public:
    SceneNode(const string& nodeName = "Node") 
        : name(nodeName), 
          active(true), 
          parent(nullptr),
          mesh(nullptr) {
    }

    virtual ~SceneNode() {
    }

    // Name
    const string& getName() const { return name; }
    void setName(const string& newName) { name = newName; }

    // state
    bool isActive() const { return active; }
    void setActive(bool state) { active = state; }

    // Transform
    Transform& getTransform() { return transform; }
    const Transform& getTransform() const { return transform; }

    // Hierarchy
    SceneNode* getParent() const { return parent; }
    
    void setParent(SceneNode* newParent) {
        if (parent == newParent) return;
        
        if (parent) {
            parent->removeChild(this);
        }
        
        parent = newParent;
        
        if (parent) {
            parent->addChild(this);
        }
        
        transform.markDirty();
    }

    void addChild(SceneNode* child) {
        if (!child || child == this) return;
        
        auto it = find(children.begin(), children.end(), child);
        if (it != children.end()) return;
        
        children.push_back(child);
        child->parent = this;
        child->transform.markDirty();
    }

    void removeChild(SceneNode* child) {
        auto it = find(children.begin(), children.end(), child);
        if (it != children.end()) {
            (*it)->parent = nullptr;
            children.erase(it);
        }
    }

    const vector<SceneNode*>& getChildren() const {
        return children;
    }

    size_t getChildCount() const {
        return children.size();
    }

    SceneNode* getChild(size_t index) const {
        if (index < children.size()) {
            return children[index];
        }
        return nullptr;
    }

    SceneNode* findChild(const string& childName) const {
        for (auto child : children) {
            if (child->name == childName) {
                return child;
            }
            
            SceneNode* found = child->findChild(childName);
            if (found) {
                return found;
            }
        }
        return nullptr;
    }

    // Mesh
    void setMesh(Mesh* newMesh) {
        mesh = newMesh;
    }

    Mesh* getMesh() const {
        return mesh;
    }

    bool hasMesh() const {
        return mesh != nullptr;
    }

    // Update
    virtual void update(float deltaTime, const glm::mat4& parentWorldMatrix) {
        if (!active) return;
        
        // world transform
        transform.updateWorldMatrix(parentWorldMatrix);
        
        // children
        for (auto child : children) {
            child->update(deltaTime, transform.getWorldMatrix());
        }
    }

    // Render
    virtual void render(const Camera& camera) {
        if (!active) return;
        
        // mesh
        if (mesh) {
            glm::mat4 modelMatrix = transform.getWorldMatrix();
            mesh->draw(camera, modelMatrix);
        }
        
        // children
        for (auto child : children) {
            child->render(camera);
        }
    }


    int getDepth() const {
        int depth = 0;
        const SceneNode* current = parent;
        while (current) {
            depth++;
            current = current->parent;
        }
        return depth;
    }

    void printHierarchy(int indent = 0) const {
        for (int i = 0; i < indent; i++) {
            cout << "  ";
        }
        cout << "- " << name;
        if (!active) cout << " (inactive)";
        if (mesh) cout << " [mesh]";
        cout << " pos(" << transform.getWorldPosition().x << ", " 
             << transform.getWorldPosition().y << ", " 
             << transform.getWorldPosition().z << ")";
        cout << endl;
        
        for (auto child : children) {
            child->printHierarchy(indent + 1);
        }
    }
};

#endif // SCENE_NODE