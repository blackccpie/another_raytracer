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
                // TODO-AM : should work!!
                //if(shape.mesh.num_face_vertices.size() != 3)
                //    throw std::logic_error("only tri-mesh are supported for now! "+std::to_string(shape.mesh.num_face_vertices.size()));
                num_triangles += shape.mesh.num_face_vertices.size();
            }

            std::cout << "mesh number of shapes:    " << parse_data.shapes.size() << '\n';
            std::cout << "mesh number of triangles: " << num_triangles << '\n';
            
            return true;
        }

        hittable_list build() {
            hittable_list triangles;
            
            auto capsule_texture = std::make_shared<image_texture>(ressources::models_path+"capsule/capsule.jpg");

            auto get_vertice_by_index = [this]<typename T>(T index) {
                const rapidobj::Array<float>& positions = parse_data.attributes.positions;
                return vec3{
                    positions[static_cast<size_t>(3*index)+0UL],
                    positions[static_cast<size_t>(3*index)+1UL],
                    positions[static_cast<size_t>(3*index)+2UL]};
            };

            auto get_texcoord_by_index = [this]<typename T>(T index) -> std::pair<double,double> {
                const rapidobj::Array<float>& texcoords = parse_data.attributes.texcoords;
                return {texcoords[static_cast<size_t>(2*index)+0UL],
                        texcoords[static_cast<size_t>(2*index)+1UL]};
            };
            
            for (const auto& shape : parse_data.shapes) {
                const rapidobj::Array<rapidobj::Index>& indices = shape.mesh.indices;
                const rapidobj::Array<std::int32_t>& material_ids = shape.mesh.material_ids;
                
                //std::cout << "shape: " << indices.size() << std::endl;
                
                for(size_t i=0; i<indices.size()/3; ++i) {
                    if(!parse_data.materials.empty()) {
                        rapidobj::Material m = parse_data.materials[static_cast<size_t>(material_ids[i])];
                        const auto Ka = m.ambient;
                        const auto Kd = m.diffuse;
                        //const auto Ks = m.specular;
                        const auto map_Kd = m.diffuse_texname;
                        if(!map_Kd.empty()) {
                            //std::cout << "material: " << map_Kd << std::endl;

                            auto [u1,v1] = get_texcoord_by_index(indices[3*i + 0].texcoord_index);
                            auto [u2,v2] = get_texcoord_by_index(indices[3*i + 1].texcoord_index);
                            auto [u3,v3] = get_texcoord_by_index(indices[3*i + 2].texcoord_index);

                            auto triangle_texture = std::make_shared<barycentric_image_texture>(
                                std::make_pair(u1,v1),
                                std::make_pair(u2,v2),
                                std::make_pair(u3,v3),
                                capsule_texture
                            );
                            triangles.add(std::make_shared<triangle>(
                                get_vertice_by_index(indices[3*i + 0].position_index), // first vertice
                                get_vertice_by_index(indices[3*i + 1].position_index), // second vertice
                                get_vertice_by_index(indices[3*i + 2].position_index), // third vertice
                                std::make_shared<lambertian>(triangle_texture)));
                        }
                        else {
                            triangles.add(std::make_shared<triangle>(
                                get_vertice_by_index(indices[3*i + 0].position_index), // first vertice
                                get_vertice_by_index(indices[3*i + 1].position_index), // second vertice
                                get_vertice_by_index(indices[3*i + 2].position_index), // third vertice
                                std::make_shared<lambertian>(color(Ka[0]+Kd[0], Ka[1]+Kd[1], Ka[2]+Kd[2]))));
                        }
                    }
                    else {
                        triangles.add(std::make_shared<triangle>(
                            get_vertice_by_index(indices[3*i + 0].position_index), // first vertice
                            get_vertice_by_index(indices[3*i + 1].position_index), // second vertice
                            get_vertice_by_index(indices[3*i + 2].position_index), // third vertice
                            std::make_shared<lambertian>(color::random())));
                    }
                }
            }

            std::cout << "hittable list successfully built from mesh description (" << triangles.size() << " hittables)" << std::endl;
            
            return triangles;
        }

    private:
        rapidobj::Result parse_data;
};

#endif
