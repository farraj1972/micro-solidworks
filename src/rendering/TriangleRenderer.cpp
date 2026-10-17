#include "rendering/TriangleRenderer.h"
#include <glad/gl.h>
#include <cmath>
#include <stdexcept>
#include <vector>
namespace microsw::rendering {
class TriangleRenderer::Impl { public: GLuint vao{},vbo{};std::size_t count{};Impl(){
 if(!GLAD_GL_VERSION_3_3)throw std::runtime_error{"TriangleRenderer requires OpenGL 3.3"};
 GLint previousVao{},previousBuffer{};glGetIntegerv(GL_VERTEX_ARRAY_BINDING,&previousVao);glGetIntegerv(GL_ARRAY_BUFFER_BINDING,&previousBuffer);
 glGenVertexArrays(1,&vao);glGenBuffers(1,&vbo);glBindVertexArray(vao);glBindBuffer(GL_ARRAY_BUFFER,vbo);
 glEnableVertexAttribArray(0);glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,6*sizeof(float),nullptr);
 glEnableVertexAttribArray(1);glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,6*sizeof(float),reinterpret_cast<void*>(3*sizeof(float)));
 glBindVertexArray(static_cast<GLuint>(previousVao));glBindBuffer(GL_ARRAY_BUFFER,static_cast<GLuint>(previousBuffer));
 }~Impl(){glDeleteVertexArrays(1,&vao);glDeleteBuffers(1,&vbo);}};
TriangleRenderer::TriangleRenderer():impl_{std::make_unique<Impl>()}{} TriangleRenderer::~TriangleRenderer()=default;
void TriangleRenderer::setVertices(std::span<const LitVertex> vertices){if(vertices.size()%3)throw std::invalid_argument{"Triangle vertices must be triples"};std::vector<float>d;d.reserve(vertices.size()*6);for(const auto&v:vertices)for(auto x:{v.position.x(),v.position.y(),v.position.z(),v.normal.x(),v.normal.y(),v.normal.z()}){if(!std::isfinite(x))throw std::invalid_argument{"Triangle data must be finite"};d.push_back(static_cast<float>(x));}glBindBuffer(GL_ARRAY_BUFFER,impl_->vbo);glBufferData(GL_ARRAY_BUFFER,static_cast<GLsizeiptr>(d.size()*sizeof(float)),d.data(),GL_DYNAMIC_DRAW);impl_->count=vertices.size();}
void TriangleRenderer::draw()const{if(!impl_->count)return;glBindVertexArray(impl_->vao);glDrawArrays(GL_TRIANGLES,0,static_cast<GLsizei>(impl_->count));}
std::size_t TriangleRenderer::vertexCount()const noexcept{return impl_->count;}
}
