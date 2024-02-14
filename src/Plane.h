#include "Skybox.h"
#define NUMBER_SAND_PATCHES 900





class Plane{

private:

    Parser * parser ;
    Skybox * skybox ;
    Shader * shader ;
    float time, fov ;
    progressbar * bar ;
    std::vector<float> patch_indices ;
    std::vector<unsigned int> textures ;
    std::vector<std::string> file_paths ;
    int num_progress_bars, number_sand_patches ;
    glm::mat4 models[NUMBER_SAND_PATCHES], projection, view ;
    glm::vec3 light_color, camera, light_position, camera_front ;
    unsigned int vao, vbo, ebo, shader_program, number_of_indices, model_vbo, patch_indices_vbo ;
    glm::mat4 lightprojection, light_view ; 

public:

    Plane( glm::mat4 view, glm::mat4 projection, glm::vec3 c, progressbar * b, std::vector<std::string> texture_paths, std::string file_path, glm::mat4 lightproj, glm::mat4 lightview ) ;
    void set( Skybox * skybox ) ;
    void generate_buffers() ;
    void blend_albedo() ;
    void shade( unsigned int shadow_shader_program ) ;
    void shade( Shadow * shadow ) ;
    void bind_vertices( std::vector<float> vertices ) ;
    void bind_indices( std::vector<unsigned int> indices ) ;
    void set_uniform_variables() ;
    void bind_models() ;
    void set_models() ;
    float compute_index(int i, int j) ;
    bool is_in_view( glm::vec3 pos ) ;
    void update_view( glm::vec3 camera, glm::mat4 v ) ;
    void load_textures() ;
    void activate_textures() ;
    void generate_error( std::string msg ) ;
    void update_progress_bar( int num_bars ) ;
    std::vector<float> get_mesh() ;
    ~Plane() ;

} ;
