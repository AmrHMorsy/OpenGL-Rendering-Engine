#include "Plane.h"
#include "../external/Include/stb_image.h"
#define EPSILON 1.4



Plane::Plane( glm::mat4 v, glm::mat4 proj, glm::vec3 c, progressbar * b, std::vector<std::string> texture_paths, std::string file_path, glm::mat4 lightproj, glm::mat4 lightview ): num_progress_bars(10),light_position({0.484394f, 7.5f, 0.416144f}), light_color({500.0f, 500.0f, 500.0f}), view(v), projection(proj), camera(c), file_paths(texture_paths), number_sand_patches(0), bar(b), lightprojection(lightproj), light_view(lightview)
{
    parser = new Parser(bar) ;
    parser->parse( file_path ) ;
    shader = new Shader( "../external/Shaders/Plane/Plane_vs.vs", "../external/Shaders/Plane/Plane_fs.fs", "../external/Shaders/Plane/Plane_tcs.tcs", "../external/Shaders/Plane/Plane_tes.tes") ;
    glPatchParameteri(GL_PATCH_VERTICES, 3);
    generate_buffers() ;
    load_textures() ;
    shader_program = shader->get_shader_program() ;
    glUseProgram(shader_program) ;
    set_models() ;
    bind_vertices( parser->get_vertices() ) ;
    bind_models() ;
    bind_indices( parser->get_vertices_faces() ) ;
    set_uniform_variables() ;
}

void Plane::set( Skybox * skybox )
{
    this->skybox = skybox ;
}

void Plane::generate_buffers()
{
    textures.resize(file_paths.size()) ;
    glGenTextures(file_paths.size(),textures.data()) ;
    glGenVertexArrays(1, &vao) ;
    glGenBuffers(1, &vbo) ;
    glGenBuffers(1, &ebo) ;
    glGenBuffers(1, &model_vbo);
    glGenBuffers(1, &patch_indices_vbo);
    update_progress_bar(10) ;
}

void Plane::shade( unsigned int shadow_shader_program )
{
    glBindVertexArray(vao) ;
    glUniformMatrix4fv(glGetUniformLocation(shadow_shader_program, "model"), 1, GL_FALSE, &models[0][0][0]) ;
    glDrawElements(GL_TRIANGLES, number_of_indices, GL_UNSIGNED_INT, 0 ) ;
    glBindVertexArray(0) ;
}

void Plane::shade( Shadow * shadow )
{
    glUseProgram(shader_program) ;
    activate_textures() ;
    shadow->activate_texture() ;
    glUniformMatrix4fv(glGetUniformLocation(shader_program, "model"), 1, GL_FALSE, &models[0][0][0]) ;
    glBindVertexArray(vao) ;
    glDrawElementsInstanced(GL_PATCHES, number_of_indices, GL_UNSIGNED_INT, 0, number_sand_patches) ;
    glBindVertexArray(0) ;
}

void Plane::bind_vertices( std::vector<float> vertices )
{
    glBindVertexArray(vao) ;
    glBindBuffer(GL_ARRAY_BUFFER, vbo) ;
    glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(float), vertices.data(), GL_STATIC_DRAW) ;
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (8*sizeof(float)), (void*)0) ;
    glEnableVertexAttribArray(0) ;
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, (8*sizeof(float)), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, (8*sizeof(float)), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);
    update_progress_bar(10) ;
}

void Plane::bind_models()
{
    glBindVertexArray(vao) ;
    glBindBuffer(GL_ARRAY_BUFFER, patch_indices_vbo);
    glBufferData(GL_ARRAY_BUFFER, number_sand_patches * sizeof(float), patch_indices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);
    glEnableVertexAttribArray(3);
    glVertexAttribDivisor(3, 1);
    glBindBuffer(GL_ARRAY_BUFFER, model_vbo);
    glBufferData(GL_ARRAY_BUFFER, number_sand_patches * sizeof(glm::mat4), &models[0], GL_STATIC_DRAW);
    for (int i = 0; i < 4; i++){
        glVertexAttribPointer(4+i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4) * i));
        glEnableVertexAttribArray(4+i);
        glVertexAttribDivisor(4+i, 1);
    }
    glBindVertexArray(0) ;
}

void Plane::bind_indices( std::vector<unsigned int> indices )
{
    glBindVertexArray(vao) ;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo) ;
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(int), indices.data(), GL_DYNAMIC_DRAW) ;
    glBindVertexArray(0) ;
    number_of_indices = indices.size() ;
    update_progress_bar(8) ;
}

void Plane::set_uniform_variables()
{
    glUniform1i(glGetUniformLocation(shader_program, "albedo_map"), 0);
    glUniform1i(glGetUniformLocation(shader_program, "ao_map"), 1);
    glUniform1i(glGetUniformLocation(shader_program, "normal_map"), 2);
    glUniform1i(glGetUniformLocation(shader_program, "roughness_map"), 3);
    glUniform1i(glGetUniformLocation(shader_program, "displacement_map"), 4);
    glUniform1i(glGetUniformLocation(shader_program, "irradiance_map"), 5) ;
    glUniform1i(glGetUniformLocation(shader_program, "prefilter_map"), 6) ;
    glUniform1i(glGetUniformLocation(shader_program, "brdf_integration_map"), 7) ;
    glUniform1i(glGetUniformLocation(shader_program, "metallic_map"), 8) ;
    glUniform1i(glGetUniformLocation(shader_program, "shadow_map"), 10) ;
    glUniformMatrix4fv(glGetUniformLocation(shader_program, "projection"), 1, GL_FALSE, &projection[0][0]) ;
    glUniform3fv(glGetUniformLocation(shader_program, "light_color"), 1, &light_color[0]) ;
    glUniform3fv(glGetUniformLocation(shader_program, "light_position"), 1, &light_position[0]) ;
    glUniform3fv(glGetUniformLocation(shader_program, "camera"), 1, &camera[0]) ;
    glUniformMatrix4fv(glGetUniformLocation(shader_program, "light_view"), 1, GL_FALSE, &light_view[0][0]) ;
    glUniformMatrix4fv(glGetUniformLocation(shader_program, "light_projection"), 1, GL_FALSE, &lightprojection[0][0]) ;

    update_progress_bar(6) ;
}

bool Plane::is_in_view( glm::vec3 pos )
{
    glm::vec3 ocean_to_camera = glm::normalize(pos-camera);
    float angleBetween = acos(glm::dot(ocean_to_camera, camera_front)) ;
    float check_angle = fov / 2 ;
    return true ;
    return angleBetween < (check_angle+EPSILON);
}

void Plane::set_models()
{
    patch_indices.clear() ;
    number_sand_patches = 0 ;
    float x, z ;
    int s = std::sqrt(NUMBER_SAND_PATCHES) ;
    for(int i = 0; i < s; i++) {
        z = (i - s / 2) * 2.0f ;
        for(int j = 0; j < s; j++) {
            x = (j - s / 2) * 2.0f ;
            if( is_in_view(glm::vec3(x,0.0f,z)) ){
                models[number_sand_patches] =  glm::translate(glm::mat4(1.0), glm::vec3(x, 0.0f, z)) ;
                patch_indices.push_back(compute_index(i,j)) ;
                number_sand_patches++ ;
            }
        }
    }
}

float Plane::compute_index(int i, int j)
{
    if( i % 2 == 0 ){
        if( j % 2 == 0 )
            return 0.0f ;
        else
            return 1.0f ;
    }
    else{
        if( j % 2 == 0 )
            return 2.0f ;
        else
            return 3.0f ;
    }
}

void Plane::update_view( glm::vec3 camera, glm::mat4 v )
{
    this->camera = camera ;
    glUseProgram(shader_program) ;
    glUniform3fv(glGetUniformLocation(shader_program, "camera"), 1, &camera[0]) ;
    glUniformMatrix4fv(glGetUniformLocation(shader_program, "view"), 1, GL_FALSE, &v[0][0]) ;
}

void Plane::load_textures()
{
    GLenum format ;
    int w, h, nc ;
    unsigned char * data ;
    for( int i = 0 ; i < file_paths.size() ; i++ ){
        glBindTexture(GL_TEXTURE_2D, textures[i]) ;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT) ;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT) ;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR) ;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR) ;
        stbi_set_flip_vertically_on_load(true) ;
        data = stbi_load(file_paths[i].c_str(), &w, &h, &nc, 0);
        format = (nc==1)? GL_RED : (nc==3)? GL_RGB : (nc==4) ? GL_RGBA : GL_RGB ;
        if (data){
            glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, data) ;
            glGenerateMipmap(GL_TEXTURE_2D) ;
        }
        else
            generate_error("Failure To Load Texture") ;
        stbi_image_free(data);
    }
    update_progress_bar(6) ;
}

void Plane::update_progress_bar( int num_bars )
{
    if( num_bars > num_progress_bars )
        return ;
    for( int i = 0 ; i < num_bars  ; i++ )
        bar->update() ;
    num_progress_bars -= num_bars ;
}

void Plane::activate_textures()
{
    for( int i = 0 ; i < textures.size()-1 ; i++ ){
        glActiveTexture(GL_TEXTURE0+i) ;
        glBindTexture(GL_TEXTURE_2D, textures[i]) ;
    }
    skybox->activate_irradiance() ;
    glActiveTexture(GL_TEXTURE8) ;
    glBindTexture(GL_TEXTURE_2D, textures[textures.size()-1]) ;
}

std::vector<float> Plane::get_mesh()
{
    return parser->get_vertices() ; 
}

void Plane::generate_error( std::string msg )
{
    std::cout << msg << std::endl ;
    exit(1) ;
}

Plane::~Plane()
{
    delete shader ;
    shader = NULL ;
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    glDeleteBuffers(textures.size(), textures.data());
}
