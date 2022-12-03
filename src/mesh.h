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
            
            size_t num_triangles = 0;

            for (const auto& shape : parse_data.shapes) {
                //if(shape.mesh.num_face_vertices.size() != 3)
                //    throw std::logic_error("only tri-mesh are supported for now! "+std::to_string(shape.mesh.num_face_vertices.size()));
                num_triangles += shape.mesh.num_face_vertices.size();
            }

            std::cout << "mesh number of shapes:    " << parse_data.shapes.size() << '\n';
            std::cout << "mesh number of triangles: " << num_triangles << '\n';
            
            return true;
        };
        hittable_list build() {
            hittable_list triangles;
            
            auto get_vertice_by_index = [this](size_t index) {
                const rapidobj::Array<float>& positions = parse_data.attributes.positions;
                return vec3{positions[3*index+0],
                            positions[3*index+1],
                            positions[3*index+2]};
            };
            
            for (const auto& shape : parse_data.shapes) {
                const rapidobj::Array<rapidobj::Index>& indices = shape.mesh.indices;
                const rapidobj::Array<std::int32_t>& material_ids = shape.mesh.material_ids;
                
                //std::cout << "shape: " << indices.size() << std::endl;
                
                for(size_t i=0; i<indices.size()/3; ++i) {
                    rapidobj::Material m = parse_data.materials[material_ids[i]];
                    const auto Ka = m.ambient;
                    const auto Kd = m.diffuse;
                    const auto Ks = m.specular;
                    triangles.add(make_shared<triangle>(
                        get_vertice_by_index(indices[3*i + 0].position_index), // first vertice
                        get_vertice_by_index(indices[3*i + 1].position_index), // second vertice
                        get_vertice_by_index(indices[3*i + 2].position_index), // third vertice
                        make_shared<phong>(color(Ka[0], Ka[1], Ka[2]),
                                           color(Kd[0], Kd[1], Kd[2]),
                                           color(Ks[0], Ks[1], Ks[2]),
                                           1)));
                        //make_shared<lambertian>(color(Kd[0], Kd[1], Kd[2]))));
                        //make_shared<lambertian>(color::random())));
                }
            }
            
            return triangles;
        }
    private:
    rapidobj::Result parse_data;
};

#endif
