#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include <string>
#include "Mesh.h"
#include "Sphere.h"
#include "Square.h"

#include <GL/glut.h>

Vec3 i_ambient = Vec3(0.1f, 0.1f, 0.1f);
Vec3 i_diffuse = Vec3(0.7f, 0.7f, 0.7f);
Vec3 i_specular = Vec3(0.5f, 0.5f, 0.5f);

enum LightType {

	LightType_Spherical,
	LightType_Quad

};

struct Light {

	Vec3 material;
	float ambientIntensity;
	float diffuseIntensity;
	float specularIntensity;
	bool isInCamSpace;
	LightType type;

	Vec3 pos;
	float radius;

	Mesh quad;

	float powerCorrection;

	Light() : powerCorrection(1.0) {}

};

struct RaySceneIntersection {

	bool intersectionExists;
	unsigned int typeOfIntersectedObject;
	unsigned int objectIndex;
	float t;
	RayTriangleIntersection rayMeshIntersection;
	RaySphereIntersection raySphereIntersection;
	RaySquareIntersection raySquareIntersection;
	RaySceneIntersection() : intersectionExists(false), t(FLT_MAX) {}

};

class Scene {

	std::vector<Mesh> meshes;
	std::vector<Sphere> spheres;
	std::vector<Square> squares;
	std::vector<Light> lights;

	public:

		Scene() {}

		void draw() {

			// iterer sur l'ensemble des objets, et faire leur rendu :
			for( unsigned int It = 0 ; It < meshes.size() ; ++It ) {
				Mesh const & mesh = meshes[It];
				mesh.draw();
			}
			for( unsigned int It = 0 ; It < spheres.size() ; ++It ) {
				Sphere const & sphere = spheres[It];
				sphere.draw();
			}
			for( unsigned int It = 0 ; It < squares.size() ; ++It ) {
				Square const & square = squares[It];
				square.draw();
			}

		}

		RaySceneIntersection computeIntersection(Ray const & ray) {

			RaySceneIntersection result;
			result.intersectionExists = false;
			result.objectIndex = -1;
			result.typeOfIntersectedObject = -1;
			result.t = FLT_MAX;

			int meshesCount = meshes.size();
			for(int i = 0; i < meshesCount; i++) {
				RayTriangleIntersection tmp = meshes[i].intersect(ray);
				if(tmp.intersectionExists && result.t > tmp.t) {
					result.intersectionExists = true;
					result.objectIndex = i;
					result.typeOfIntersectedObject = 0;
					result.t = tmp.t;
					result.rayMeshIntersection = tmp;
				}
			}

			int spheresCount = spheres.size();
			for(int i = 0; i < spheresCount; i++) {
				RaySphereIntersection tmp = spheres[i].intersect(ray);
				if(tmp.intersectionExists && result.t > tmp.t) {
					result.intersectionExists = true;
					result.objectIndex = i;
					result.typeOfIntersectedObject = 1;
					result.t = tmp.t;
					result.raySphereIntersection = tmp;
				}
			}

			int squaresCount = squares.size();
			for(int i = 0; i < squaresCount; i++) {
				RaySquareIntersection tmp = squares[i].intersect(ray);
				if(tmp.intersectionExists && result.t > tmp.t) {
					result.intersectionExists = true;
					result.objectIndex = i;
					result.typeOfIntersectedObject = 2;
					result.t = tmp.t;
					result.raySquareIntersection = tmp;
				}
			}

			return result;

		}

		Vec3 rayTraceRecursive( Ray ray , int NRemainingBounces ) {

			//TODO RaySceneIntersection raySceneIntersection = computeIntersection(ray);
			RaySceneIntersection result = computeIntersection(ray);

			if(NRemainingBounces == 0) {
				Vec3 intersection;
				switch(result.typeOfIntersectedObject) {
					case 0:
						intersection = result.rayMeshIntersection.intersection;
						break;
					case 1:
						intersection = result.raySphereIntersection.intersection;
						break;
					case 2:
						intersection = result.raySquareIntersection.intersection;
						break;
					default:
						std::cerr << "rayTrace::Error, invalid object type\n";
						exit(EXIT_FAILURE);
				}
				if(result.intersectionExists && (intersection - (ray.origin() + ray.direction())).length() < ray.direction().length()) return Vec3(-1.f, -1.f, -1.f);
				return Vec3(1.f, 1.f, 1.f);
			} else if(NRemainingBounces == 1) {
				if(!result.intersectionExists) return Vec3(0.f, 0.f, 0.f);
				Vec3 intersection, normal, color;
				Vec3 k_ambient, k_diffuse, k_specular;
				float shininess;

				switch(result.typeOfIntersectedObject) {
					case 0:
						k_ambient = meshes[result.objectIndex].material.ambient_material;
						k_diffuse = meshes[result.objectIndex].material.diffuse_material;
						k_specular = meshes[result.objectIndex].material.specular_material;
						color = meshes[result.objectIndex].material.color;
						shininess = meshes[result.objectIndex].material.shininess;
						intersection = result.rayMeshIntersection.intersection;
						normal = result.rayMeshIntersection.normal;
						break;
					case 1:
						k_ambient = spheres[result.objectIndex].material.ambient_material;
						k_diffuse = spheres[result.objectIndex].material.diffuse_material;
						k_specular = spheres[result.objectIndex].material.specular_material;
						color = spheres[result.objectIndex].material.color;
						shininess = spheres[result.objectIndex].material.shininess;
						intersection = result.raySphereIntersection.intersection;
						normal = result.raySphereIntersection.normal;
						break;
					case 2:
						k_ambient = squares[result.objectIndex].material.ambient_material;
						k_diffuse = squares[result.objectIndex].material.diffuse_material;
						k_specular = squares[result.objectIndex].material.specular_material;
						color = squares[result.objectIndex].material.color;
						shininess = squares[result.objectIndex].material.shininess;
						intersection = result.raySquareIntersection.intersection;
						normal = result.raySquareIntersection.normal;
						break;
					default:
						std::cerr << "rayTrace::Error, invalid object type\n";
						exit(EXIT_FAILURE);
				}

				Vec3 ambient, diffuse, specular;
				ambient = Vec3(0.f, 0.f, 0.f);
				diffuse = Vec3(0.f, 0.f, 0.f);
				specular = Vec3(0.f, 0.f, 0.f);

				int lightsCount = lights.size();
				int litCheck = lightsCount;
				for(int i = 0; i < lightsCount; i++) {
					Vec3 tmp = rayTraceRecursive(Ray(0.0001f * normal + intersection, lights[i].pos - intersection), 0);
					if(tmp[0] < 0.f) litCheck--;
				}
				// if(litCheck == 0) return Vec3(0.f, 0.f, 0.f);

				for(int i = 0; i < lightsCount; i++) {

					ambient += lights[i].ambientIntensity * k_ambient;

					Vec3 lightVector = lights[i].pos - intersection;
					lightVector.normalize();

					float d_angle = Vec3::dot(lightVector, normal);

					diffuse += lights[i].diffuseIntensity * k_diffuse * d_angle * lights[i].material;

					Vec3 reflectedVector = 2*Vec3::dot(lightVector, normal)*normal - lightVector;

					float s_angle = Vec3::dot(reflectedVector, -1*ray.direction());
					if(s_angle < 0) s_angle = 0;
					else s_angle = powf(s_angle, shininess);

					specular += lights[i].specularIntensity * k_specular * s_angle * lights[i].material;

				}

				color = Vec3::clamp(color * (ambient + (litCheck == 0 ? 0.f : 1.f)*(diffuse + specular)), 0.f, 1.f);
				// color = Vec3::clamp(color * (ambient + diffuse + specular), 0.f, 1.f);

				return color;
			}

		}


		Vec3 rayTrace( Ray const & rayStart ) {

			//TODO appeler la fonction recursive
			Vec3 color = rayTraceRecursive(rayStart, 1);
			return color;

		}

		void setup_single_sphere(Vec3 color = Vec3(0.f, 0.f, 0.f), Vec3 pos = Vec3(0.f, 0.f, 0.f), float radius = 1.f) {

			meshes.clear();
			spheres.clear();
			squares.clear();
			lights.clear();

			{
				lights.resize(lights.size() + 1);
				Light &light = lights[lights.size() - 1];
				light.pos = Vec3(-5., 5., 5.);
				light.radius = 2.5f;
				light.powerCorrection = 2.f;
				light.type = LightType_Spherical;
				light.material = Vec3(1., 1., 1.);
				light.ambientIntensity = 1.f;
				light.diffuseIntensity = 1.f;
				light.specularIntensity = 1.f;
				light.isInCamSpace = false; 
			}

			{
				spheres.resize(spheres.size() + 1);
				Sphere &s = spheres[spheres.size() - 1];
				s.m_center = pos;
				s.m_radius = radius;
				s.build_arrays();
				s.material.type = Material_Mirror;
				s.material.color = color;
				s.material.ambient_material = i_ambient;
        		s.material.diffuse_material = i_diffuse;
        		s.material.specular_material = i_specular;
				s.material.shininess = 20;
			}

		}

		void setup_two_spheres(Vec3 color1 = Vec3(0.f, 0.f, 0.f), Vec3 pos1 = Vec3(0.f, 0.f, 0.f), float radius1 = 1.f, Vec3 color2 = Vec3(0.f, 0.f, 0.f), Vec3 pos2 = Vec3(0.f, 0.f, 0.f), float radius2 = 1.f) {

			meshes.clear();
			spheres.clear();
			squares.clear();
			lights.clear();

			{
				lights.resize(lights.size() + 1);
				Light &light = lights[lights.size() - 1];
				light.pos = Vec3(-5., 5., 5.);
				light.radius = 2.5f;
				light.powerCorrection = 2.f;
				light.type = LightType_Spherical;
				light.material = Vec3(1., 1., 1.);
				light.ambientIntensity = 1.f;
				light.diffuseIntensity = 1.f;
				light.specularIntensity = 1.f;
				light.isInCamSpace = false; 
			}

			{
				spheres.resize(spheres.size() + 1);
				Sphere &s = spheres[spheres.size() - 1];
				s.m_center = pos1;
				s.m_radius = radius1;
				s.build_arrays();
				s.material.type = Material_Mirror;
				s.material.color = color1;
				s.material.ambient_material = i_ambient;
        		s.material.diffuse_material = i_diffuse;
        		s.material.specular_material = i_specular;
				s.material.shininess = 20;
			}

			{
				spheres.resize(spheres.size() + 1);
				Sphere &s = spheres[spheres.size() - 1];
				s.m_center = pos2;
				s.m_radius = radius2;
				s.build_arrays();
				s.material.type = Material_Mirror;
				s.material.color = color2;
				s.material.ambient_material = i_ambient;
        		s.material.diffuse_material = i_diffuse;
        		s.material.specular_material = i_specular;
				s.material.shininess = 20;
			}

		}

		void setup_single_square() {

			meshes.clear();
			spheres.clear();
			squares.clear();
			lights.clear();

			{
				lights.resize(lights.size() + 1);
				Light & light = lights[lights.size() - 1];
				light.pos = Vec3(-5., 5., 5.);
				light.radius = 2.5f;
				light.powerCorrection = 2.f;
				light.type = LightType_Spherical;
				light.material = Vec3(1., 1., 1.);
				light.ambientIntensity = 1.f;
				light.diffuseIntensity = 1.f;
				light.specularIntensity = 1.f;
				light.isInCamSpace = false;
			}

			{
				squares.resize(squares.size() + 1);
				Square & s = squares[squares.size() - 1];
				s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0., 0.), Vec3(0., 1., 0.), 2., 2.);
				s.build_arrays();
				s.material.color = Vec3(0.8, 0.8, 0.8);
				s.material.ambient_material = i_ambient;
        		s.material.diffuse_material = i_diffuse;
        		s.material.specular_material = i_specular;
				s.material.shininess = 20;
			}

		}

	void setup_cornell_box() {

		meshes.clear();
		spheres.clear();
		squares.clear();
		lights.clear();

		{
			lights.resize(lights.size() + 1);
			Light &light = lights[lights.size() - 1];
			light.pos = Vec3(0.0, 1.5, 0.0);
			light.radius = 2.5f;
			light.powerCorrection = 2.f;
			light.type = LightType_Spherical;
			light.material = Vec3(1., 1., 1.);
			light.ambientIntensity = 1.f;
			light.diffuseIntensity = 1.f;
			light.specularIntensity = 1.f;
			light.isInCamSpace = false;
		}

		{ // Back Wall
			squares.resize(squares.size() + 1);
			Square &s = squares[squares.size() - 1];
			s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0., 0.), Vec3(0., 1., 0.), 2., 2.);
			s.scale(Vec3(2., 2., 1.));
			s.translate(Vec3(0., 0., -2.));
			s.build_arrays();
			s.material.color = Vec3(0., 1., 1.);
			s.material.ambient_material = i_ambient;
        	s.material.diffuse_material = i_diffuse;
        	s.material.specular_material = i_specular;
			s.material.shininess = 16;
		}

		{ // Left Wall

			squares.resize(squares.size() + 1);
			Square &s = squares[squares.size() - 1];
			s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0., 0.), Vec3(0., 1., 0.), 2., 2.);
			s.scale(Vec3(2., 2., 1.));
			s.translate(Vec3(0., 0., -2.));
			s.rotate_y(90);
			s.build_arrays();
			s.material.color = Vec3(1., 0., 0.);
			s.material.ambient_material = i_ambient;
        	s.material.diffuse_material = i_diffuse;
        	s.material.specular_material = i_specular;
			s.material.shininess = 16;
		}

		{ // Right Wall
			squares.resize(squares.size() + 1);
			Square &s = squares[squares.size() - 1];
			s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0., 0.), Vec3(0., 1., 0.), 2., 2.);
			s.translate(Vec3(0., 0., -2.));
			s.scale(Vec3(2., 2., 1.));
			s.rotate_y(-90);
			s.build_arrays();
			s.material.color = Vec3(0.0, 1.0, 0.0);
			s.material.ambient_material = i_ambient;
        	s.material.diffuse_material = i_diffuse;
        	s.material.specular_material = i_specular;
			s.material.shininess = 16;
		}

		{ // Floor
			squares.resize(squares.size() + 1);
			Square &s = squares[squares.size() - 1];
			s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0., 0.), Vec3(0., 1., 0.), 2., 2.);
			s.translate(Vec3(0., 0., -2.));
			s.scale(Vec3(2., 2., 1.));
			s.rotate_x(-90);
			s.build_arrays();
			s.material.color = Vec3(1.0, 1.0, 1.0);
			s.material.ambient_material = i_ambient;
        	s.material.diffuse_material = i_diffuse;
        	s.material.specular_material = i_specular;
			s.material.shininess = 16;
		}

		{ // Ceiling
			squares.resize(squares.size() + 1);
			Square &s = squares[squares.size() - 1];
			s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0., 0.), Vec3(0., 1., 0.), 2., 2.);
			s.translate(Vec3(0., 0., -2.));
			s.scale(Vec3(2., 2., 1.));
			s.rotate_x(90);
			s.build_arrays();
			s.material.color = Vec3(1.0, 0.0, 1.0);
			s.material.ambient_material = i_ambient;
        	s.material.diffuse_material = i_diffuse;
        	s.material.specular_material = i_specular;
			s.material.shininess = 16;
		}

		{ // Front Wall
			squares.resize(squares.size() + 1);
			Square &s = squares[squares.size() - 1];
			s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0., 0.), Vec3(0., 1., 0.), 2., 2.);
			s.translate(Vec3(0., 0., -2.));
			s.scale(Vec3(2., 2., 1.));
			s.rotate_y(180);
			s.build_arrays();
			s.material.color = Vec3(1.0, 1.0, 1.0);
			s.material.ambient_material = i_ambient;
        	s.material.diffuse_material = i_diffuse;
        	s.material.specular_material = i_specular;
			s.material.shininess = 16;
		}

		{ // GLASS Sphere

			spheres.resize(spheres.size() + 1);
			Sphere &s = spheres[spheres.size() - 1];
			s.m_center = Vec3(1.0, -1.25, 0.5);
			s.m_radius = 0.75f;
			s.build_arrays();
			s.material.type = Material_Mirror;
			s.material.color = Vec3(1., 0., 0.);
			s.material.ambient_material = i_ambient;
        	s.material.diffuse_material = i_diffuse;
        	s.material.specular_material = i_specular;
			s.material.shininess = 16;
			s.material.transparency = 1.0;
			s.material.index_medium = 1.4;
		}


		{ // MIRRORED Sphere
			spheres.resize(spheres.size() + 1);
			Sphere &s = spheres[spheres.size() - 1];
			s.m_center = Vec3(-1.0, -1.25, -0.5);
			s.m_radius = 0.75f;
			s.build_arrays();
			s.material.type = Material_Glass;
			s.material.color = Vec3(1., 1., 0.);
			s.material.ambient_material = i_ambient;
        	s.material.diffuse_material = i_diffuse;
        	s.material.specular_material = i_specular;
			s.material.shininess = 16;
			s.material.transparency = 0.;
			s.material.index_medium = 0.;
		}

	}

};

#endif
