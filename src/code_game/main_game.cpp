#define NANOSVG_IMPLEMENTATION	// Expands implementation
#include "3dparty/nanosvg/src/nanosvg.h"
#define NANOSVGRAST_IMPLEMENTATION
#include "3dparty/nanosvg/src/nanosvgrast.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_IMPLEMENTATION
#include "..\common\gltf_loader.h"


#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>
#include <algorithm>
#include <conio.h>
#include <direct.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include "..\common\debugging.h"
#include "..\common\renderable.h"
#include "..\common\shaders.h"
#include "..\common\simple_shapes.h"
#include "..\common\carousel\carousel.h"
#include "..\common\carousel\carousel_to_renderable.h"
#include "..\common\frame_buffer_object.h";
#include "..\common\view_manipulator.h";



#include "..\common\carousel\carousel_loader.h"


#include "..\common\matrix_stack.h"
#include "..\common\intersection.h"
#include "..\common\trackball.h"
#include "..\common\texture.h"



int width, height;

trackball tb[2];
int curr_tb;

/* projection matrix*/
glm::mat4 proj[3];
int curr_proj;

/* view matrix */
std::vector<glm::mat4> views;
int curr_view;


renderable r_quad, r_cube, r_cube_skybox;
renderable fram;
shape s_cube, s_track;

float scaling_factor = 1.0;


float delta_f = 0.f;
float last_frame_time = 0.f;
glm::vec3 camera_center = glm::vec3(0, 1.f, 1.5);
glm::vec3 camera_front = glm::vec3(0.f, -1.f, -1.5f);
glm::vec3 camera_up = glm::vec3(0, 1.f, 0.f);
glm::vec3 initial_camera_center;
glm::vec3 initial_camera_front;
glm::vec3 initial_camera_up;
float camera_speed = 1.f;
float last_x = 400;
float last_y = 400;



bool first_mouse = true;

std::vector<float> track_text_coord;
std::vector<float> terrain_text_coord;
std::vector<float> track_normals_coord;
std::vector<float> terrain_normals_coord;

//variabili per switchare pov al cameramen
std::vector<glm::mat4> cameramen_views;
int num_views = 2;
int curr_cameraman;
//variabili da aggiustare con imGui
glm::vec3 original_adjust_eye = glm::vec3(0.f, 0.011f, 0.f);
float cameraman_z = 0.f;
glm::vec3 adjust_eye = original_adjust_eye;

//variabili per rendering scena
box3 berlina_bbox, cameraman_bbox, tree_bbox, lampione_bbox;
std::vector <renderable> berlina_obj, cameraman_obj, tree_obj, lampione_obj;
renderable r_lamps, r_trees, r_track, r_terrain;
matrix_stack stack;
int num_alberi;

texture track_texture, track_normal_map;
texture terrain_texture, fanali_texture;
texture skybox;
shader basic_shader, depth_shader, full_screen_quad_shader, flat_shader, blur_shader;
glm::mat4 flip_matrix = glm::rotate(glm::mat4(1.f), glm::radians(180.f), glm::vec3(0.f, 1.f, 0.f));
glm::mat4 flip_orizzontale_matrix = glm::rotate(glm::mat4(1.f), glm::radians(180.f), glm::vec3(1.f, 0.f, 0.f));
glm::mat4 flip =  flip_matrix* flip_orizzontale_matrix;

frame_buffer_object shadow_map_fbo,blur_variance_map_fbo;
int shadow_map_size_x;
int shadow_map_size_y;
float distance_light;
float initial_distance_light;
float light_near = 0.05f;
float depth_bias;
float adjustable_distance_light_tuning = 0.05f;
glm::mat4 LDir_view;
glm::mat4 LDir_light_matrix;
bool shadow_map_in_basso_a_sx = false;
bool disegna_light_frustum_si = false;
bool adjustable_distance_light = true;
static float k_plane_approx = 0.5;

glm::mat4 fanali_view[10];
glm::mat4 fanali_proj;
float inclinazione_fanali_z = -20.f;
float inclinazione_fanali_x = 0.f;
float inclinazione_fanali_y = -25.5f;
float near_fanali = 0.028f;
float far_fanali = 1.6f;
float fanali_pos_z = 0.f;
float fanali_pos_y = 0.85f;
float fanali_pos_x = 0.f;
bool disegna_frustum_fanali = false;

float far_lampioni = 1.3f;
float near_lampioni = 0.01f;
float theta_in = glm::radians(22.f);
float theta_out = glm::radians(48.f);
float blur_lamp = 3.f;
float pos_luce_lampione_x = -0.3f;
float pos_luce_lampione_y = 0.5f;
float pos_luce_lampione_z = 0.f;
float raggio_lampioni = 0.004f;
float lamp_shadow_x = -23.75f;
float lamp_shadow_y = -33.f;
float lamp_shadow_z = -19.f;
//segno la posizione delle macchine per attivare le shadow map dei lampioni
glm::vec4 pos_macchine[10];
glm::mat4 lampioni_proj;
glm::mat4 lampioni_light_view[10];
glm::mat4 lampioni_light_matrix[10];
int attiva_shadow_map_lampione[10];
bool mostra_frustum_lampioni = false;
frame_buffer_object shadow_map_lampioni_fbo[10];
int lampione_shadow_map_in_basso_a_sx = -1;
float lamp_bias = 0.0005f;





GLuint track_nm_id, track_texture_id, terrain_texture_id;
GLuint fanali_texture_id;


view_manipulator view_man;
bool first_person_mode;
int approx_normal_terreno = 0;

void setup_camera_vectors();
void switch_to_cameraman();
void setup_cameramen_views();
void crea_texture_coordinates_track(const race& r);
void crea_normali_track(const race& r);
void crea_normali_terreno(renderable& terreno, const race& r);
void crea_texture_coordinates_terreno(renderable& terreno, const race& r);
void carica_texture();
static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void model_bind(matrix_stack& stack, float scale, std::vector<renderable>& obj, box3 &bbox, int texture_unit, shader sh, glm::mat4 model_transformation);
void gui_setup();
void disegna_scena(shader& sh, const race& r, bool backface_culling);
void disegna_cubo();
void disegna_light_frustum(glm::mat4& light_matrix);
void disegna_full_screen_quad();
void disegna_texture_fsq(GLint tex_id);
void blur_texture(GLint tex_id);
glm::mat4 crea_light_matrix(glm::mat4& view, float distance_light);
void processInput(GLFWwindow* window);





	int main(int argc, char** argv)
	{
		race r;
		
		carousel_loader::load("small_test.svg", "terrain_256.png",r);
		
		//add 10 cars
		for (int i = 0; i < 10; ++i)		
			r.add_car();

		GLFWwindow* window;

		/* Initialize the library */
		if (!glfwInit())
			return -1;
		width = 800;
		height = 800;
		/* Create a windowed mode window and its OpenGL context */
		window = glfwCreateWindow(width, height, "CarOusel", NULL, NULL);
		if (!window)
		{
			glfwTerminate();
			return -1;
		}
		/* declare the callback functions on mouse events */
		if (glfwRawMouseMotionSupported())
			glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
		
		glfwSetCursorPosCallback(window, cursor_position_callback);
		glfwSetMouseButtonCallback(window, mouse_button_callback);
		glfwSetScrollCallback(window, scroll_callback);
		glfwSetKeyCallback(window, key_callback);

		/* Make the window's context current */
		glfwMakeContextCurrent(window);

		glewInit();
		
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		ImGui_ImplOpenGL3_Init();
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		
		printout_opengl_glsl_info();


		gltf_loader taxi_gltfL, cameraman_gltfL, tree_gltfL, lampione_gltfL;

		
		std::string path = "skybox/";
		skybox.load_cubemap(path + "right.jpg", path + "left.jpg",
			path + "top.jpg", path + "bottom.jpg",
			path + "front.jpg", path + "back.jpg", 11);
	

		cameraman_gltfL.load_to_renderable("modelli/cameraman.glb", cameraman_obj, cameraman_bbox);
		tree_gltfL.load_to_renderable("modelli/low_poly_pine_tree.glb", tree_obj, tree_bbox);
		taxi_gltfL.load_to_renderable("modelli/low_poly_berlina.glb", berlina_obj, berlina_bbox);
		lampione_gltfL.load_to_renderable("modelli/low_poly_psx_street_lamp.glb", lampione_obj, lampione_bbox);

		/*
		GLint maxTextureUnits;
		glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextureUnits);

		std::cout << "Numero massimo di texture units disponibili: " << maxTextureUnits << std::endl;
		//32
		GLint maxVertexUniforms, maxFragmentUniforms, maxVaryingFloats;
		glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &maxVertexUniforms);
		glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, &maxFragmentUniforms);
		glGetIntegerv(GL_MAX_VARYING_FLOATS, &maxVaryingFloats);

		std::cout << "Numero massimo di componenti uniform nel vertex shader: " << maxVertexUniforms << std::endl;
		std::cout << "Numero massimo di componenti uniform nel fragment shader: " << maxFragmentUniforms << std::endl;
		//1600
		std::cout << "Numero massimo di variabili varying: " << maxVaryingFloats << std::endl;
		//128
		*/

		fram = shape_maker::frame();
		
		r_cube_skybox = shape_maker::cube();

		r_cube = shape_maker::cube();
		shape_maker::cube(s_cube);
		s_cube.compute_edges();
		s_cube.to_renderable(r_cube);
		
		r_quad = shape_maker::quad();
		//crea renderable e texture coordinates terreno
		
		r_terrain.create();
		game_to_renderable::to_heightfield(r, r_terrain);
		crea_texture_coordinates_terreno(r_terrain, r);
		crea_normali_terreno(r_terrain, r);
		r_terrain.add_vertex_attribute<float>(&terrain_text_coord[0], unsigned int(terrain_text_coord.size()), 4, 2);
		r_terrain.add_vertex_attribute<float>(&terrain_normals_coord[0], unsigned int(terrain_normals_coord.size()), 2, 3);
		
		
		//crea renderable e texture coordinates track
		//renderable r_track;
		r_track.create();
		game_to_renderable::to_track(r, r_track);
		crea_texture_coordinates_track(r);
		crea_normali_track(r);



		r_track.add_vertex_attribute<float>(&track_text_coord[0], unsigned int(track_text_coord.size()), 4, 2);
		r_track.add_vertex_attribute<float>(&track_normals_coord[0], unsigned int(track_normals_coord.size()), 2, 3);
		

		carica_texture();
		
		
		r_trees.create();
		game_to_renderable::to_tree(r, r_trees);

		
		r_lamps.create();
		game_to_renderable::to_lamps(r, r_lamps);
		
		
		basic_shader.create_program("shaders/basic.vert", "shaders/basic.frag");
		depth_shader.create_program("shaders/depthmap.vert", "shaders/depthmap.frag");
		full_screen_quad_shader.create_program("shaders/fsq.vert", "shaders/fsq.frag");
		flat_shader.create_program("shaders/flat.vert", "shaders/flat.frag");
		
		blur_shader.create_program("shaders/fsq.vert", "shaders/blur.frag");
		/* use the program shader "program_shader" */
		glUseProgram(basic_shader.program);
		
		/* define the viewport  */
		glViewport(0, 0, width, height);

		tb[0].reset();
		tb[0].set_center_radius(glm::vec3(0, 0, 0), 1.f);
		tb[1].set_center_radius(glm::vec3(0, 0, 0), 1.f);
		curr_tb = 0;
		

		proj[0] = glm::perspective(glm::radians(45.f), 1.f, 1.f, 10.f);
		proj[1] = glm::perspective(glm::radians(55.f), 1.f, 0.01f, 5.5f); //proj per cameraman
		proj[2] = glm::perspective(glm::radians(45.f), 1.f, 0.01f, 6.f);
		view_man.reset();
		curr_proj = 0;

		curr_view = 0;
		num_views = r.cameramen().size() + 1;
		setup_camera_vectors();
		//view = glm::lookAt(glm::vec3(0, 1.f, 1.5), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.0, 1.f, 0.f));
		views[0] = glm::lookAt(camera_center, camera_center + camera_front, camera_up);
		
		initial_camera_center = camera_center;
		initial_camera_front = camera_front;
		initial_camera_up = camera_up;
		
		
		glUniform3f(basic_shader["uLightColor"], 0.8f, 0.8f, 0.8f);
		glUniform1f(depth_shader["uPlaneApprox"], k_plane_approx);
		glUniform1i(basic_shader["uNormalMapTracks"], 6);
		glUniform1i(basic_shader["uSkybox"], 11);
		

		r.start(11,0,0,600);
		r.update();
		//variabili per la luce
		distance_light = 0.24f;
		initial_distance_light = distance_light;
		shadow_map_size_x = 1024;
		shadow_map_size_y = 1024;
		depth_bias = 0.007f;

		num_alberi = r.trees().size();

		glEnable(GL_DEPTH_TEST);
		
		shadow_map_fbo.create(shadow_map_size_x,shadow_map_size_y,true);
		blur_variance_map_fbo.create(shadow_map_size_x, shadow_map_size_y, true);
		glm::vec3 pos_lamp[10];
		for (int i = 0; i < 10; ++i) {
			pos_lamp[i] = r.lamps()[i].pos;
			float s = 1.f / r.bbox().diagonal();
			glm::vec3 c = r.bbox().center();
			glm::mat4 scala = (glm::scale(glm::mat4(1.f), glm::vec3(s)));
			glm::mat4 trasla = (glm::translate(glm::mat4(1.f), -c));
			glm::mat4 trasla_again = glm::translate(glm::mat4(1.f), pos_lamp[i] + glm::vec3(0.f, 3.5f, 0.f)) * glm::translate(glm::mat4(1.f), glm::vec3(pos_luce_lampione_x, pos_luce_lampione_y, pos_luce_lampione_z));
			pos_lamp[i] = glm::vec3(scala * trasla * trasla_again * glm::vec4(0.f, 0.f, 0.f, 1.0));
			shadow_map_lampioni_fbo[i].create(shadow_map_size_x, shadow_map_size_y, true);
		}

		
		int uniformLampShadowMap[10];
		for (int i = 0; i < 10; ++i) {
			uniformLampShadowMap[i] = 12 + i;
		}

		glUniform1iv(basic_shader["uLampioniShadowMaps"], 10,&uniformLampShadowMap[0]);
		
		
		
		 /* Loop until the user closes the window */
		while (!glfwWindowShouldClose(window))
		{
			/* Render here */
			glClearColor(0.3f, 0.3f, 0.3f, 1.f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
			gui_setup();
			

			//delta_time per modulare velocità in base al frameRate
			float current_time = glfwGetTime();
			delta_f = current_time - last_frame_time;
			last_frame_time = current_time;

			views[0] = glm::lookAt(camera_center, camera_center + camera_front, camera_up);
			
			if(first_person_mode) views[0] = view_man.apply_to_view(views[0]);

			fanali_proj = glm::frustum(-0.01f, 0.01f, -0.005f, 0.005f, near_fanali, far_fanali);
			lampioni_proj = glm::frustum(-0.01f, 0.01f, -0.005f, 0.005f, near_lampioni, far_lampioni);
			

			glUniformMatrix4fv(basic_shader["uFanaliProj"], 1, GL_FALSE, &fanali_proj[0][0]);
			glUniformMatrix4fv(basic_shader["uView"], 1, GL_FALSE, &views[curr_view][0][0]);
			glUniformMatrix4fv(basic_shader["uProj"], 1, GL_FALSE, &proj[curr_proj][0][0]);
			
			glUniform1f(basic_shader["uThetaIn"], theta_in);
			glUniform1f(basic_shader["uThetaOut"], theta_out);
			glUniform1f(basic_shader["uBlurLamp"], blur_lamp);
			glUniform1f(basic_shader["uRaggioLampioni"], raggio_lampioni);
			glUniform1i(basic_shader["uApprossimaNormaliTerreno"], approx_normal_terreno);
			r.update();
			//disegna scena in lightSpace
			
			glBindFramebuffer(GL_FRAMEBUFFER, shadow_map_fbo.id_fbo);
			glUseProgram(depth_shader.program);
			glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			glViewport(0, 0, shadow_map_size_x, shadow_map_size_y);
			
			glm::vec4 curr_Ldir = glm::vec4(r.sunlight_direction(),0.f);
			
			if (adjustable_distance_light) {
				float cos = glm::dot(r.sunlight_direction(), glm::vec3(0.f, 1.f, 0.f));// *adjustable_distance_light_tuning;
				if(cos >0.7f)
					distance_light = initial_distance_light * (1 / cos);
			}
			
			
			LDir_view = glm::lookAt(r.sunlight_direction() * distance_light, glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, -1.f));
			if (curr_view == 0) {
				LDir_view = LDir_view * inverse(tb[0].matrix());
			}
			

			
			
			LDir_light_matrix = crea_light_matrix(LDir_view, distance_light);
			glUniformMatrix4fv(depth_shader["uLightMatrix"], 1, GL_FALSE, &LDir_light_matrix[0][0]);
			
			disegna_scena(depth_shader, r,false);


			//disegno scena per shadow map lampioni

			unsigned int count_lampioni_attivi = 0;
			for (unsigned int i = 0; i < 10; ++i) {


				if (attiva_shadow_map_lampione[i] == 1) {
					count_lampioni_attivi+=1;

					glBindFramebuffer(GL_FRAMEBUFFER, shadow_map_lampioni_fbo[i].id_fbo);
					glUseProgram(depth_shader.program);
					glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
					glViewport(0, 0, shadow_map_size_x, shadow_map_size_y);

					glm::vec3 direzione_vista;
					if (i < 5) direzione_vista = glm::normalize(glm::vec3(lamp_shadow_x, lamp_shadow_y, lamp_shadow_z));
					else direzione_vista = glm::normalize(glm::vec3(-lamp_shadow_x, lamp_shadow_y, -lamp_shadow_z));

					lampioni_light_view[i] = glm::lookAt(pos_lamp[i], pos_lamp[i] + direzione_vista, camera_up);

					lampioni_light_matrix[i] = lampioni_proj * lampioni_light_view[i];
					if (curr_view == 0) lampioni_light_matrix[i] = lampioni_light_matrix[i] * inverse(tb[0].matrix());
					//glm::mat4 prova = proj[curr_proj] * views[curr_view];
					glUniformMatrix4fv(depth_shader["uLightMatrix"], 1, GL_FALSE, &lampioni_light_matrix[i][0][0]);
					disegna_scena(depth_shader, r, false);
					glActiveTexture(GL_TEXTURE12 + i);
					//va da 12 a 31

					glBindTexture(GL_TEXTURE_2D, shadow_map_lampioni_fbo[i].id_tex);

				}
				attiva_shadow_map_lampione[i] = 0;
			}
			if (count_lampioni_attivi > 7) std::cout << "---" << count_lampioni_attivi << "----\n";
			

			glUseProgram(basic_shader.program);
			glUniformMatrix4fv(basic_shader["uLampLightMatrix"], 10, GL_FALSE, &lampioni_light_matrix[0][0][0]);
			

			//disegno scena normalmente


			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glUseProgram(basic_shader.program);
			glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			glViewport(0, 0, width, height); 
			glUniform1f(basic_shader["uBias"], depth_bias);
			glUniform1f(basic_shader["uLampBias"], lamp_bias);
			glUniformMatrix4fv(basic_shader["uLightMatrix"], 1, GL_FALSE, &LDir_light_matrix[0][0]);
			glUniform1i(basic_shader["uShadowMap"], 3);
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, shadow_map_fbo.id_tex);
			
			disegna_scena(basic_shader, r,true);
			

			
			
			if (shadow_map_in_basso_a_sx) {
				glViewport(0, 0, 200, 200);
				glDisable(GL_DEPTH_TEST);
				disegna_texture_fsq(shadow_map_fbo.id_tex);
				glEnable(GL_DEPTH_TEST);
				glViewport(0, 0, width, height);
			}
			if (lampione_shadow_map_in_basso_a_sx != -1) {
				glViewport(0, 0, 200, 200);
				glDisable(GL_DEPTH_TEST);
				disegna_texture_fsq(shadow_map_lampioni_fbo[lampione_shadow_map_in_basso_a_sx].id_tex);
				glEnable(GL_DEPTH_TEST);
				glViewport(0, 0, width, height);
			}
			if (disegna_light_frustum_si) {
				disegna_light_frustum(LDir_light_matrix);

			}

			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


			if(first_person_mode) processInput(window);
			/* Swap front and back buffers */
			glfwSwapBuffers(window);

			/* Poll for and process events */
			glfwPollEvents();
		}
		glUseProgram(0);
		glfwTerminate();
		return 0;
	}

	
	void disegna_scena(shader& sh,const race &r, bool backface_culling) {
		
		glUseProgram(sh.program);
		stack.load_identity();
		stack.push();
		if (curr_view == 0)
			stack.mult(tb[0].matrix());
		
		if (sh.has_uniform("uObject")) glUniform1i(sh["uObject"], 2);
		if (backface_culling) disegna_cubo();
		
		glUniformMatrix4fv(sh["uModel"], 1, GL_FALSE, &stack.m()[0][0]);


		if (sh.has_uniform("uSunlightDir")) glUniform3f(sh["uSunlightDir"], r.sunlight_direction().x, r.sunlight_direction().y, r.sunlight_direction().z);

		float s = 1.f / r.bbox().diagonal();
		glm::vec3 c = r.bbox().center();
		glm::mat4 scala = (glm::scale(glm::mat4(1.f), glm::vec3(s)));
		glm::mat4 trasla = (glm::translate(glm::mat4(1.f), -c));
		stack.mult(glm::scale(glm::mat4(1.f), glm::vec3(s)));
		stack.mult(glm::translate(glm::mat4(1.f), -c));

		


		glDisable(GL_CULL_FACE);
		//DISEGNO TERRENO
		glDepthRange(0.1, 1); //originale era 0.01,1
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, terrain_texture_id);
		glUniformMatrix4fv(sh["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
		if(sh.has_uniform("uColor")) glUniform3f(sh["uColor"], 1, 1, 1.0); //colore originale

		if(sh.has_uniform("uObject")) glUniform1i(sh["uObject"], 1);
		if(sh.has_uniform("uTerrainTexture")) glUniform1i(sh["uTerrainTexture"], 0);
		
		r_terrain.bind();

		
		glDrawElements(r_terrain().mode, r_terrain().count, r_terrain().itype, NULL);
		glDepthRange(0.0, 1);
		if (sh.has_uniform("uObject")) glUniform1i(sh["uObject"], 2);

		
		
		
		//DISEGNO TRACK
		
		r_track.bind();
		if (sh.has_uniform("uObject")) glUniform1i(sh["uObject"], 0);
		glPointSize(3.0);
		
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, track_texture_id);
		if (sh.has_uniform("uTrackTexture")) glUniform1i(sh["uTrackTexture"], 0);
		if (sh.has_uniform("uNormalMapTracks")) glUniform1i(sh["uNormalMapTracks"],1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, track_nm_id);
		
		glDrawArrays(GL_TRIANGLE_STRIP, 0, r_track.vn);
		glPointSize(1.0);
		if (sh.has_uniform("uObject")) glUniform1i(sh["uObject"], 2);
		
		
		
		
		if (backface_culling) {
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
		}
		
		/*DISEGNO MACCHINE*/
		float berlina_scale = 1 / berlina_bbox.diagonal();
		for (unsigned int ic = 0; ic < r.cars().size(); ++ic) {
			stack.push();

			stack.mult(r.cars()[ic].frame);
			pos_macchine[ic] = stack.m() * glm::vec4(0.f, 0.f, 0.f, 1.f);
			
			glUniformMatrix4fv(sh["uModel"], 1, GL_FALSE, &glm::translate(glm::mat4(1.f), glm::vec3(pos_macchine[ic]))[0][0]);
			/*Disegno frame macchine*/
			
			if(sh.has_uniform("uObject")) glUniform1i(sh["uObject"], 3);
			if (sh.has_uniform("uColor")) glUniform3f(sh["uColor"], -0.7, 0.7, 0.3);
			if (sh.has_uniform("uModelTexture")) glUniform1i(sh["uModelTexture"], 0);
			
			
			/*aggiusto posizione e dimensione macchine*/
			
			stack.mult(glm::scale(glm::mat4(1.f), glm::vec3(2.f, 2.f, 2.f)));
			stack.mult(glm::translate(glm::mat4(1.f), glm::vec3(0, 0.1, 0.0)));
			model_bind(stack, berlina_scale, berlina_obj, berlina_bbox, 0, sh,flip_matrix);
			if (backface_culling) {
				glActiveTexture(GL_TEXTURE2);
				glBindTexture(GL_TEXTURE_2D, fanali_texture_id);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
				if (sh.has_uniform("uFanaliTexture")) glUniform1i(sh["uFanaliTexture"], 2);
				//creo matrice vista e proiezione per fanali

				glm::vec3 fanali_center = glm::vec3(stack.m() * glm::vec4(fanali_pos_x, fanali_pos_y, fanali_pos_z, 1.0));
				glm::vec3 fanali_target = glm::vec3(stack.m() * glm::vec4(fanali_pos_x + inclinazione_fanali_x,fanali_pos_y + inclinazione_fanali_y,fanali_pos_z + inclinazione_fanali_z,1.0));
				fanali_view[ic] = glm::lookAt(fanali_center, fanali_target, glm::vec3(0.f, 1.f, 0.f));
				
				glUniformMatrix4fv(sh["uFanaliView"], 10, GL_FALSE, &fanali_view[0][0][0]);
				if (disegna_frustum_fanali) {
					glm::mat4 fanali_mat = fanali_proj * fanali_view[ic];
					disegna_light_frustum(fanali_mat);
				}
				glUseProgram(sh.program);

				
			}
			stack.pop();
		}

		if (sh.has_uniform("uObject")) glUniform1i(sh["uObject"], 2);

		//DISEGNO CAMERAMAN
		float cameraman_scale = 1 / cameraman_bbox.diagonal();

		for (unsigned int ic = 0; ic < r.cameramen().size(); ++ic) {
			stack.push();
			stack.mult(r.cameramen()[ic].frame);
			stack.mult(glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.5f, 0.f)));
			stack.mult(glm::scale(glm::mat4(1.f), glm::vec3(1, 1, 1)));

			cameramen_views[ic] = stack.m();

			glUniformMatrix4fv(sh["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
			if (sh.has_uniform("uObject")) glUniform1i(sh["uObject"], 4);
			
			model_bind(stack, cameraman_scale, cameraman_obj, cameraman_bbox, 0, sh, glm::rotate(glm::mat4(1.f),glm::radians(180.f),glm::vec3(0.f,0.f,1.f)));

			stack.pop();
		}
		if (curr_view != 0) setup_cameramen_views();

		if(sh.has_uniform("uObject")) glUniform1i(sh["uObject"], 2);


		//DISEGNO ALBERI
		stack.push();

		float tree_scale = 1.f / tree_bbox.diagonal();
		if (sh.has_uniform("uObject")) glUniform1i(sh["uObject"], 5);
		for (unsigned int it = 0; it < num_alberi; ++it) {
			stack.push();
			stack.mult(glm::translate(glm::mat4(1.f), r.trees()[it].pos + glm::vec3(0.f, 1.f, 0.f)));
			stack.mult(glm::scale(glm::mat4(1.f), glm::vec3(7.f, 7.f, 7.f)));
			
			model_bind(stack, tree_scale, tree_obj, tree_bbox, 0, sh,glm::mat4(1.f));
			stack.pop();
		}
		stack.pop();
		if (sh.has_uniform("uObject")) glUniform1i(sh["uObject"], 2);
		glUniformMatrix4fv(sh["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
		
		//DISEGNO LAMPIONI
		r_lamps.bind();
		//Sono 10 lampioni
		if (sh.has_uniform("uObject")) glUniform1i(sh["uObject"], 6);
		float lamp_scale = 1.f / lampione_bbox.diagonal();
		glm::vec3 lamp_light_point[10];
		glm::vec3 lamp_light_target[10];
		

		for (unsigned int il = 0; il < 10; ++il) {
			stack.push();
			stack.mult(glm::translate(glm::mat4(1.f), r.lamps()[il].pos + glm::vec3(0.f, 1.5f, 0.f)));
			stack.mult(glm::scale(glm::mat4(1.f), glm::vec3(5.f, 5.f, 5.f)));
			
			stack.push();

			
			if(il<5) stack.mult(glm::translate(glm::mat4(1.f), glm::vec3(pos_luce_lampione_x, pos_luce_lampione_y, pos_luce_lampione_z)));
			else stack.mult(glm::translate(glm::mat4(1.f), glm::vec3(-pos_luce_lampione_x*2.f, pos_luce_lampione_y, pos_luce_lampione_z)));
			
			lamp_light_point[il] = glm::vec3(stack.m() * glm::vec4(0.f, 0.f, 0.f, 1.f));

			lamp_light_target[il] = glm::vec3(stack.m()*glm::translate(glm::mat4(1.f), glm::vec3(0.f, -pos_luce_lampione_y/8 , 0.f)) * glm::vec4(0.f,0.f,0.f, 1.f));
			
			stack.pop();

			
			if (backface_culling) {
				
				for (int i = 0; i < 10; ++i) {
					
					glm::vec3 lamp_macchina = glm::vec3(pos_macchine[i]) - lamp_light_point[il];
					if (glm::dot(lamp_macchina, lamp_macchina) < raggio_lampioni ) {
						attiva_shadow_map_lampione[il] = 1;
					}
				}
				if(attiva_shadow_map_lampione[il]!=0 && mostra_frustum_lampioni) disegna_light_frustum(lampioni_light_matrix[il]);
				glUseProgram(sh.program);
			}
			
			
			if (sh.has_uniform("uModelTexture")) glUniform1i(sh["uModelTexture"], 0);
			
			if (il < 5) model_bind(stack, lamp_scale, lampione_obj, lampione_bbox, 0, sh,flip );
			else model_bind(stack, lamp_scale, lampione_obj, lampione_bbox, 0, sh, glm::mat4(1.f));
			stack.pop();
		}
		if (sh.has_uniform("uLampAttivo")) glUniform1iv(sh["uLampAttivo"], 10, &attiva_shadow_map_lampione[0]);
		if (sh.has_uniform("uLampPointLight")) glUniform3fv(sh["uLampPointLight"],10,&lamp_light_point[0][0]);
		if (sh.has_uniform("uLampLightTarget")) glUniform3fv(sh["uLampLightTarget"], 10, &lamp_light_target[0][0]);
		

		

		if (sh.has_uniform("uColor")) glUniform3f(sh["uColor"], -1.f, 1.0f, 0.f);
		

		stack.pop();
	}


	void setup_camera_vectors() {
		cameramen_views.resize(num_views-1);
		views.resize(num_views);
	}

	void setup_cameramen_views() {
		
		glm::vec3 eye = glm::vec3(cameramen_views[curr_cameraman][3]);
		glm::vec3 cameramen_view_direction = glm::normalize(glm::vec3(-cameramen_views[curr_cameraman][2]));
		glm::vec3 target = eye + cameramen_view_direction;
		views[curr_view] = glm::lookAt(eye + adjust_eye, target, camera_up);
		
	}


	void crea_normali_track(const race& r) {
		int track_normals_coord_size = r.t().curbs[0].size() * 2 * 3;
		track_normals_coord.resize(track_normals_coord_size);
		//soluzione base, approssimo tutte le normali con (0.f,1.f,0.f)
		//sol migliore (?): calcolo d = curbs[1][i]-curbs[0][i], calcolo l'angolo tra
		//d e x e ruoto (0,1,0) di quell'angolo
		for (int i = 0; i + 3 < track_normals_coord_size; i = i + 3) {
			track_normals_coord[i] = 0.f;
			track_normals_coord[i + 1] = 1.f;
			track_normals_coord[i + 2] = 0.f;
		}

	}

	glm::vec3 to_vec3(int i, std::vector<float>& v) {
		return glm::vec3(v[i * 3], v[i * 3 + 1], v[i * 3 + 2]);
	}

	void crea_normali_terreno(renderable& terreno, const race& r) {
		
		std::vector<unsigned int > buffer_id;

		const unsigned int& Z = static_cast<unsigned int>(r.ter().size_pix[1]);
		const unsigned int& X = static_cast<unsigned int>(r.ter().size_pix[0]);

		terrain ter = r.ter();

		std::vector<float>   hf3d;

		for (unsigned int iz = 0; iz < Z; ++iz)
			for (unsigned int ix = 0; ix < X; ++ix) {
				hf3d.push_back(ter.rect_xz[0] + (ix / float(X)) * ter.rect_xz[2]);
				hf3d.push_back(r.ter().hf(ix, iz));
				hf3d.push_back(ter.rect_xz[1] + (iz / float(Z)) * ter.rect_xz[3]);
			}
		for (unsigned int iz = 0; iz < Z - 1; ++iz)
			for (unsigned int ix = 0; ix < X - 1; ++ix) {

				buffer_id.push_back((iz * Z) + ix);
				buffer_id.push_back((iz * Z) + ix + 1);
				buffer_id.push_back((iz + 1) * Z + ix + 1);

				buffer_id.push_back((iz * Z) + ix);
				buffer_id.push_back((iz + 1) * Z + ix + 1);
				buffer_id.push_back((iz + 1) * Z + ix);
			}

		int terrain_normals_coord_size = hf3d.size();
		int n = buffer_id.size();
		terrain_normals_coord.resize(terrain_normals_coord_size);

		for (int i = 0; i < terrain_normals_coord_size; ++i) {
			terrain_normals_coord.push_back(0.f);
		}
		
		
		
		for (int it = 0; it < n/3; ++it) {
			std::vector<glm::vec3> p;
			p.resize(3);
			p[0] = to_vec3(buffer_id[it * 3], hf3d);
			p[1] = to_vec3(buffer_id[it * 3 + 1], hf3d);
			p[2] = to_vec3(buffer_id[it * 3 + 2], hf3d);

			glm::vec3 p01 = p[1] - p[0];
			glm::vec3 p02 = p[2] - p[0];

			glm::vec3 normale = normalize(glm::cross(p02, p01));

			
			
			for (int iv = 0; iv < 3; ++iv) {
		

				terrain_normals_coord[3 * buffer_id[it * 3 + iv]] += normale.x;
				terrain_normals_coord[3 * buffer_id[it * 3 + iv] + 1] += normale.y;
				terrain_normals_coord[3 * buffer_id[it * 3 + iv] + 2] += normale.z;

			}
			
			
		}
		for (int i = 0; i < terrain_normals_coord_size; ++i) {
			//ci sono 6 triangoli che incidono per ogni vertice, eccetto i vertici agli estremi
			terrain_normals_coord[i] = terrain_normals_coord[i] / 6;
		}
		
		
	}
	void crea_texture_coordinates_track(const race& r) {
		int track_text_coord_size = r.t().curbs[0].size() * 2 * 2;
		track_text_coord.resize(track_text_coord_size);
		float j = 0;
		for (int i = 0; i + 4 < track_text_coord_size; i = i + 4) {
			track_text_coord[i] = 0;
			track_text_coord[i + 1] = j;
			track_text_coord[i + 2] = 1;
			track_text_coord[i + 3] = j;
			j = (j + 0.025f);
			
			
		}
	}
	void crea_texture_coordinates_terreno(renderable& terreno, const race& r) {
		int terreno_text_coord_size = (terreno.vn * 2);
		terrain_text_coord.resize(terreno_text_coord_size);
		const unsigned int& X = static_cast<unsigned int>(r.ter().size_pix[0]);
		float x = 0;
		float y = 0;
		for (int i = 0; i < terreno_text_coord_size - 1; i = i +2) {
			terrain_text_coord[i] = x;
			terrain_text_coord[i + 1] = y;
			x += 0.0625f;
			
			if (i % X == 0) {
				x = 0;
				y += 0.0625f;
				
			}
		}
	}

	
	void carica_texture() {
		track_texture_id = track_texture.load("street_tile.png", 0);
		terrain_texture_id = terrain_texture.load("erba.png", 0);
		track_nm_id = track_normal_map.load("normal_map.png", 1);
		fanali_texture_id = fanali_texture.load("fanali.png", 1);
	}

	void model_bind(matrix_stack &stack, float scale,std::vector<renderable> &obj, box3 &bbox, int texture_unit, shader sh, glm::mat4 model_transformation) {
		stack.push();
		stack.mult(glm::scale(glm::mat4(1.f), glm::vec3(scale, scale, scale)));
		stack.mult(glm::translate(glm::mat4(1.f), glm::vec3(-bbox.center())));
		for (unsigned int ib = 0; ib < obj.size(); ++ib) {
			obj[ib].bind();
			stack.push();
			stack.mult(obj[ib].transform);
			glActiveTexture(GL_TEXTURE0 + texture_unit);
			glBindTexture(GL_TEXTURE_2D, obj[ib].mater.base_color_texture);
			glActiveTexture(GL_TEXTURE1 + texture_unit);
			glBindTexture(GL_TEXTURE_2D, obj[ib].mater.emissive_texture);
			stack.mult(model_transformation);
			glUniformMatrix4fv(sh["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
			glDrawElements(obj[ib]().mode, obj[ib]().count, obj[ib]().itype, 0);
			glActiveTexture(GL_TEXTURE0);
			stack.pop();
		}
		stack.pop();
	}

	glm::mat4 crea_light_matrix(glm::mat4& view, float distance_light) {
		glm::mat4 orthogonal_proj = glm::ortho(-0.4f, 0.4f, -0.4f, 0.4f, light_near, distance_light*2.f);

		return orthogonal_proj * view;
	}

	void disegna_cubo() {
		//Cubo per skybox
		r_cube_skybox.bind();
		glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &glm::scale(glm::mat4(1.f), glm::vec3(2.5f, 2.5f, 2.5f))[0][0]);
		glDrawElements(r_cube().mode, r_cube().count, r_cube().itype, 0);
	}

	void disegna_light_frustum(glm::mat4& light_matrix) {
		r_cube.bind();
		stack.push();
		stack.mult(glm::inverse(light_matrix));
		glUseProgram(flat_shader.program);
		glUniformMatrix4fv(flat_shader["uView"], 1, GL_FALSE, &views[curr_view][0][0]);

		glUniformMatrix4fv(flat_shader["uProj"], 1, GL_FALSE, &proj[curr_proj][0][0]);
		glUniform3f(flat_shader["uColor"], 0.0, 0.0, 1.0);
		glUniformMatrix4fv(flat_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, r_cube.elements[1].ind);
		glDrawElements(r_cube.elements[1].mode, r_cube.elements[1].count, r_cube.elements[1].itype, 0);

		stack.mult(glm::translate(glm::mat4(1.f), glm::vec3(0, 0, -1)));
		stack.mult(glm::scale(glm::mat4(1.f), glm::vec3(1, 1, 0)));
		glUniform3f(flat_shader["uColor"], 1.0, 1.0, 0.0);
		glUniformMatrix4fv(flat_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, r_cube.elements[0].ind);
		glDrawElements(r_cube.elements[0].mode, r_cube.elements[0].count, r_cube.elements[0].itype, 0);
		stack.pop();
	}
	
	void disegna_full_screen_quad() {
		r_quad.bind();
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	}
	void disegna_texture_fsq(GLint tex_id) {
		GLint active_texture;
		glGetIntegerv(GL_ACTIVE_TEXTURE, &active_texture);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex_id);
		glUseProgram(full_screen_quad_shader.program);
		glUniform1i(full_screen_quad_shader["uTexture"], 0);
		disegna_full_screen_quad();
		
		glUseProgram(basic_shader.program);
		glActiveTexture(active_texture);

	}
	void blur_texture(GLint tex_id) {
		GLint active_texture;
		glGetIntegerv(GL_ACTIVE_TEXTURE, &active_texture);

		glBindFramebuffer(GL_FRAMEBUFFER, blur_variance_map_fbo.id_fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blur_variance_map_fbo.id_tex, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex_id);
		glUseProgram(blur_shader.program);
		glUniform1i(blur_shader["uTexture"], 0);
		glUniform2f(blur_shader["uBlur"], 0.0, 1 / blur_variance_map_fbo.h);

		disegna_full_screen_quad();

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_id, 0);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		glBindTexture(GL_TEXTURE_2D, blur_variance_map_fbo.id_tex);
		glUniform2f(blur_shader["uBlur"], 1.f / blur_variance_map_fbo.w, 0.0);
		disegna_full_screen_quad();

		glUseProgram(0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glActiveTexture(active_texture);
	}
	/* callback function called when the mouse is moving */
	static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
	{	
		if (first_person_mode) {
			view_man.mouse_move(xpos, ypos);
			
		}
		else tb[curr_tb].mouse_move(proj[curr_proj], views[curr_view], xpos, ypos);
	}

	void processInput(GLFWwindow* window) {
		glm::mat4 view_mat = glm::inverse(views[0]);
		glm::vec3 camera_front_dir = glm::normalize(-glm::vec3(view_mat[2]));

		const float cameraSpeed = camera_speed * delta_f;
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
			camera_center += cameraSpeed * camera_front_dir;
			
		}
			//camera_center += cameraSpeed * camera_front;
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
			camera_center -= cameraSpeed * camera_front_dir;
		}

		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
			camera_center -= glm::normalize(glm::cross(camera_front_dir, camera_up)) * cameraSpeed;
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
			camera_center += glm::normalize(glm::cross(camera_front_dir, camera_up)) * cameraSpeed;

		}
	}

	/* callback function called when a mouse button is pressed */
	void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
	{

		if (ImGui::GetIO().WantCaptureMouse) return;

		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
			double xpos, ypos;
			glfwGetCursorPos(window, &xpos, &ypos);
			if (first_person_mode) {
				view_man.mouse_press(xpos, ypos);
			}
			else tb[curr_tb].mouse_press(proj[curr_proj], views[curr_view], xpos, ypos);
		}
		else
			if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
				if (first_person_mode) view_man.mouse_release();
				else tb[curr_tb].mouse_release();
			}
			else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
			}

	}

	/* callback function called when a mouse wheel is rotated */
	void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
	{
		if (ImGui::GetIO().WantCaptureMouse) return;
		if (curr_tb == 0 && !first_person_mode)
			tb[0].mouse_scroll(xoffset, yoffset);
	}

	void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
		
		if (action == GLFW_PRESS) {
			
			
			switch (key) {
			case GLFW_KEY_C:
				if(!first_person_mode)
					switch_to_cameraman();
				break;
			case GLFW_KEY_P:
				if (curr_view == 0) {
					if (first_person_mode) {
						curr_proj = 0;
						camera_front = initial_camera_front;
						camera_up = initial_camera_up;
						camera_center = initial_camera_center;
						
					}
					else curr_proj = 2;

					first_person_mode = !first_person_mode;
				}
				
				
			default:
				curr_tb = 1 - curr_tb;
				
			}
		}

	}
	void switch_to_cameraman() {
		if (curr_view == 0 || curr_view == num_views - 1) {
			curr_proj = 0;
			std::cout << "---" << curr_proj << "----\n"; //per debug
		}
		curr_proj = 1;
		curr_view = (curr_view + 1) % num_views;
		curr_cameraman = curr_view - 1;
	}
	void gui_setup() {
		ImGui::BeginMainMenuBar();
		if (ImGui::BeginMenu("Camera")) {
			if (ImGui::SliderFloat("cameraman_z", &cameraman_z, -0.5f, 0.5f)) adjust_eye = original_adjust_eye + glm::vec3(-cameraman_z, 0.f, 0.f);
			if (ImGui::SliderFloat("camera speed", &camera_speed, 0.f, 2.f));
			if (ImGui::Selectable("modalita' prima persona", first_person_mode == true)) {
				first_person_mode = true;
				curr_proj = 2;

			}
			if (ImGui::Selectable("modalita' trackball", first_person_mode == false)) {
				first_person_mode = false;
				curr_proj = 0;
				camera_front = initial_camera_front;
				camera_center = initial_camera_center;
				camera_up = initial_camera_up;
			}
			if (ImGui::Selectable("Approssima normali terreno", approx_normal_terreno == 1)) approx_normal_terreno = 1;
			if (ImGui::Selectable("Non approssima normali terreno", approx_normal_terreno == 0)) approx_normal_terreno = 0;
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("ShadowMap")) {
			if (ImGui::Selectable("Mostra shadow map in basso a sinistra",shadow_map_in_basso_a_sx == true)) shadow_map_in_basso_a_sx = true;
			if (ImGui::Selectable("Nascondi shadow map in basso a sinistra", shadow_map_in_basso_a_sx == false)) shadow_map_in_basso_a_sx = false;
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Light frustum")) {
			if (ImGui::Selectable("mostra light frustum",disegna_light_frustum_si == true)) disegna_light_frustum_si = true;
			if (ImGui::Selectable("nascondi light frustum",disegna_light_frustum_si == false)) disegna_light_frustum_si = false;
			if (ImGui::Selectable("rendi light frutum adattabile",adjustable_distance_light == true)) adjustable_distance_light = true;
			if (ImGui::Selectable("rendi light frustum statico", adjustable_distance_light == false)) adjustable_distance_light = false;
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("LightFrustum parametri")) {
			if (ImGui::SliderFloat("distance_light", &initial_distance_light, 0.f, 10.f)) distance_light = initial_distance_light;
			if (ImGui::SliderFloat("bias", &depth_bias, 0.f, 0.3f));
			if (ImGui::SliderFloat("light near", &light_near, -1.f, 0.f));
			if (ImGui::SliderFloat("adjustable_distance_light_tuning", &adjustable_distance_light_tuning, 0.01f, 10.f));
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Fanali")) {

			if(ImGui::SliderFloat("near fanali", &near_fanali, -0.5,0.5));
			if(ImGui::SliderFloat("far fanali", &far_fanali, 0.01f,3.f));
			if (ImGui::SliderFloat("pos z fanali", &fanali_pos_z, -50.f, 100.f));
			if (ImGui::SliderFloat("pos y fanali", &fanali_pos_y, -0.5f, 20.f));
			if (ImGui::SliderFloat("pos x fanali", &fanali_pos_x, -10.f, 30.f));
			if (ImGui::Selectable("mostra frustum fanali", disegna_frustum_fanali == true)) disegna_frustum_fanali = true;
			if (ImGui::Selectable("nascondi frustum fanali", disegna_frustum_fanali == false)) disegna_frustum_fanali = false;
			if (ImGui::SliderFloat("inclinazione fanali y", &inclinazione_fanali_y, -100.f, 100.f));
			if (ImGui::SliderFloat("inclinazione fanali z", &inclinazione_fanali_z, -100.f, 100.f));
			if (ImGui::SliderFloat("inclinazione fanali x", &inclinazione_fanali_x, -100.f, 100.f));
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Lampioni")) {
			if (ImGui::BeginMenu("Posizione luce")) {
				if (ImGui::SliderFloat("posizione luce x", &pos_luce_lampione_x, -1.f, 1.f));
				if (ImGui::SliderFloat("posizione luce y", &pos_luce_lampione_y, -10.f, 10.f));
				if (ImGui::SliderFloat("posizione luce z", &pos_luce_lampione_z, -10.f, 10.f));
				if (ImGui::SliderFloat("theta in", &theta_in, 0, 45.f)) theta_in = glm::radians(theta_in);
				if (ImGui::SliderFloat("theta out", &theta_out, 0, 90.f)) theta_out = glm::radians(theta_out);
				if (ImGui::SliderFloat("raggio lampioni", &raggio_lampioni, 0.f, 0.5f)); //originale era 0.01f
				if (ImGui::SliderFloat("blur", &blur_lamp, 0.f, 50.f));
				ImGui::EndMenu();
			}
			
			
			
			if (ImGui::BeginMenu("Ombra lampioni")) {
				if (ImGui::SliderFloat("far lampioni", &far_lampioni, 0.f, 5.f));
				if (ImGui::SliderFloat("near lampioni", &near_lampioni, 0.f, 0.05f));
				if (ImGui::SliderFloat("posizione ombra x", &lamp_shadow_x, -50.f, 100.f));
				if (ImGui::SliderFloat("posizione ombra y", &lamp_shadow_y, -100.f, 30.f));
				if (ImGui::SliderFloat("posizione ombra z", &lamp_shadow_z, -30.f, 30.f));
				if (ImGui::Selectable("mostra frustum lampioni", mostra_frustum_lampioni == true)) mostra_frustum_lampioni = true;
				if (ImGui::Selectable("nascondi frustum lampioni", mostra_frustum_lampioni == false)) mostra_frustum_lampioni = false;
				if (ImGui::SliderFloat("bias shadow map", &lamp_bias, -0.5f, 0.3f));
				if (ImGui::Button("mostra shadow map lampione 1 in basso a sx")) {
					lampione_shadow_map_in_basso_a_sx += 1;
					if (lampione_shadow_map_in_basso_a_sx == 10) lampione_shadow_map_in_basso_a_sx = -1;
				}
				ImGui::EndMenu();
			}

			
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
	