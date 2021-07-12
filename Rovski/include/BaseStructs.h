//
// Created by 罗斌 on 2021/6/26.
//

#ifndef ROVSKI_BASESTRUCTS_H
#define ROVSKI_BASESTRUCTS_H

#include <glm/glm.hpp>
#include <array>
#include <tuple>

template<size_t Idx,class T>
constexpr size_t tuple_element_offset() {
    return static_cast<size_t>(
            reinterpret_cast<char*>(&std::get<Idx>(*reinterpret_cast<T*>(0))) - reinterpret_cast<char*>(0));
}

template<typename DataType> VkFormat GetDataFormat(){
    if constexpr(std::is_same_v<DataType, glm::vec3>){
        return VK_FORMAT_R32G32B32_SFLOAT;
    } else if constexpr(std::is_same_v<DataType, glm::vec2>) {
        return  VK_FORMAT_R32G32_SFLOAT;
    } else {
        assert(0);
    }
}

template <class ... Types>
class VertexTemp : public std::tuple<Types...> {
public:
    using DataType = std::tuple<Types...>;
    using ArrayType = std::array<VkVertexInputAttributeDescription,std::tuple_size_v<DataType>>;
    VertexTemp(std::tuple<Types...> data) : std::tuple<Types...>(data) {}
    static VkVertexInputBindingDescription getBindingDescription(){
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        bindingDescription.stride = sizeof(std::tuple<Types...>);
        return bindingDescription;
    }

    static ArrayType getVertexInputAttributeDescription() {
        ArrayType result{};
        fillDes<0>(result);
        return result;
    }
private:
    template<int index> static void fillDes(ArrayType &result) {
        if constexpr(index < std::tuple_size_v<DataType>) {
            result[index].binding = 0;
            result[index].location = index;
            result[index].format = GetDataFormat<std::tuple_element_t<index, DataType>>();
            result[index].offset = tuple_element_offset<index, DataType>();
            fillDes<index+1>(result);
        }
    }
};

using Vertex = VertexTemp<glm::vec3, glm::vec3, glm::vec2>;



/*
struct Vertex{
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

    static VkVertexInputBindingDescription getBindingDescription(){
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        bindingDescription.stride = sizeof(Vertex);
        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription,3> getVertexInputAttributeDescription() {
        std::array<VkVertexInputAttributeDescription,3> attributeDescription{};
        attributeDescription[0].binding = 0;
        attributeDescription[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescription[0].location = 0;
        attributeDescription[0].offset = offsetof(Vertex, pos);

        attributeDescription[1].binding = 0;
        attributeDescription[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescription[1].location = 1;
        attributeDescription[1].offset = offsetof(Vertex, color);

        attributeDescription[2].binding = 0;
        attributeDescription[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescription[2].location = 2;
        attributeDescription[2].offset = offsetof(Vertex, texCoord);
        return attributeDescription;
    }
};
 */

#endif //ROVSKI_BASESTRUCTS_H
