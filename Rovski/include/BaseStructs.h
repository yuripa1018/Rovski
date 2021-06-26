//
// Created by 罗斌 on 2021/6/26.
//

#ifndef ROVSKI_BASESTRUCTS_H
#define ROVSKI_BASESTRUCTS_H

#include <glm/glm.hpp>
#include <array>

struct Vertex{
    glm::vec2 pos;
    glm::vec3 color;

    static VkVertexInputBindingDescription getBindingDescription(){
        VkVertexInputBindingDescription bindingDescription;
        bindingDescription.binding = 0;
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        bindingDescription.stride = sizeof(Vertex);
        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription,2> getVertexInputAttributeDescription() {
        std::array<VkVertexInputAttributeDescription,2> attributeDescription{};
        attributeDescription[0].binding = 0;
        attributeDescription[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescription[0].location = 0;
        attributeDescription[0].offset = offsetof(Vertex, pos);

        attributeDescription[1].binding = 0;
        attributeDescription[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescription[1].location = 1;
        attributeDescription[1].offset = offsetof(Vertex, color);
        return attributeDescription;
    }
};

#endif //ROVSKI_BASESTRUCTS_H
