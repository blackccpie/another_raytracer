#ifndef MESH_H
#define MESH_H

#include "triangle.h"

#include "rapidobj.hpp"

class mesh {
    public:
        bool parse(const std::string& mesh_path) {
            parse_data = rapidobj::ParseFile(mesh_path);
            
            if(parse_data.error) {
                std::cerr << "cannot parse mesh file (error : " << parse_data.error.code.message() << ")" << std::endl;
                if (!parse_data.error.line.empty()) {
                        std::cerr << "\t -> on line " << parse_data.error.line_num << ": \"" << parse_data.error.line << "\"\n";
                    }
                return false;
            }
            
            rapidobj::Triangulate(parse_data);
            
            if(parse_data.error) {
                std::cerr << "cannot triangulate parsed mesh file" << std::endl;
                return false;
            }
            
            auto num_triangles = size_t();

            for (const auto& shape : parse_data.shapes) {
                num_triangles += shape.mesh.num_face_vertices.size();
            }

            std::cout << "mesh number of shapes:    " << parse_data.shapes.size() << '\n';
            std::cout << "mesh number of triangles: " << num_triangles << '\n';
            
            return true;
        };
        hittable_list build() {
            hittable_list triangles;
            
            // TODO-AM : temporary hardcoded
            auto lambertian_mat = make_shared<lambertian>(color(0.4, 0.2, 0.1));
            
            auto get_vertice_by_index = [this](size_t index) {
                const rapidobj::Array<float>& positions = parse_data.attributes.positions;
                return vec3{positions[3*index+0],
                            positions[3*index+1],
                            positions[3*index+2]};
            };
            
            for (const auto& shape : parse_data.shapes) {
                const rapidobj::Array<rapidobj::Index>& indices = shape.mesh.indices;
                
                for(size_t i=0; i<indices.size()/3; ++i) {
                    triangles.add(make_shared<triangle>(
                        get_vertice_by_index(indices[3*i + 0].position_index), // first vertice
                        get_vertice_by_index(indices[3*i + 1].position_index), // second vertice
                        get_vertice_by_index(indices[3*i + 2].position_index), // third vertice
                        lambertian_mat));
                }
            }
            
            return triangles;
        }
    private:
    rapidobj::Result parse_data;
};

#endif
