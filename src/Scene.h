#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include <string>
#include "Mesh.h"
#include "Sphere.h"
#include "Square.h"

#include <GL/glut.h>

enum LightType {

	LightType_Spherical,
	LightType_Quad

};

struct Light {

	Vec3 material;
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
			Vec3 color;
			return color;

		}


		Vec3 rayTrace( Ray const & rayStart ) {

			//TODO appeler la fonction recursive
			RaySceneIntersection result = computeIntersection(rayStart);
			Vec3 color, intersection, normal, res = Vec3(0.f, 0.f, 0.f);
			if(!result.intersectionExists) return Vec3(0.f, 0.f, 0.f);
			// std::cout << result.typeOfIntersectedObject << "\n";
			switch(result.typeOfIntersectedObject) {
				case 0:
					color = meshes[result.objectIndex].material.diffuse_material;
					intersection = result.rayMeshIntersection.intersection;
					normal = result.rayMeshIntersection.normal;
					break;
				case 1:
					color = spheres[result.objectIndex].material.diffuse_material;
					intersection = result.raySphereIntersection.intersection;
					normal = result.raySphereIntersection.normal;
					break;
				case 2:
					color = squares[result.objectIndex].material.diffuse_material;
					intersection = result.raySquareIntersection.intersection;
					normal = result.raySquareIntersection.normal;
					break;
				default:
					std::cerr << "rayTrace::Error, invalid object type\n";
					exit(EXIT_FAILURE);
			}
			float diffuse = 0.8f;
			float ambient = 0.2f;
			int lightsCount = lights.size();
			for(int i = 0; i < lightsCount; i++) {
				Vec3 lightVector = lights[i].pos - intersection;
				lightVector.normalize();
				float angle = Vec3::dot(lightVector, normal);
				res += color * diffuse * angle;
			}

			res += color * ambient;
			//res.normalize();
			return res;

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
				light.isInCamSpace = false; 
			}

			{
				spheres.resize(spheres.size() + 1);
				Sphere &s = spheres[spheres.size() - 1];
				s.m_center = pos;
				s.m_radius = radius;
				s.build_arrays();
				s.material.type = Material_Mirror;
				s.material.diffuse_material = color;
				s.material.specular_material = Vec3(0.2, 0.2, 0.2);
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
				light.isInCamSpace = false; 
			}

			{
				spheres.resize(spheres.size() + 1);
				Sphere &s = spheres[spheres.size() - 1];
				s.m_center = pos1;
				s.m_radius = radius1;
				s.build_arrays();
				s.material.type = Material_Mirror;
				s.material.diffuse_material = color1;
				s.material.specular_material = Vec3(0.2, 0.2, 0.2);
				s.material.shininess = 20;
			}

			{
				spheres.resize(spheres.size() + 1);
				Sphere &s = spheres[spheres.size() - 1];
				s.m_center = pos2;
				s.m_radius = radius2;
				s.build_arrays();
				s.material.type = Material_Mirror;
				s.material.diffuse_material = color2;
				s.material.specular_material = Vec3(0.2, 0.2, 0.2);
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
				light.isInCamSpace = false;
			}

			{
				squares.resize(squares.size() + 1);
				Square & s = squares[squares.size() - 1];
				s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0., 0.), Vec3(0., 1., 0.), 2., 2.);
				s.build_arrays();
				s.material.diffuse_material = Vec3(0.8, 0.8, 0.8);
				s.material.specular_material = Vec3(0.8, 0.8, 0.8);
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
			light.material = Vec3(1., 0., 0.);
			light.isInCamSpace = false;
		}

		{
			lights.resize(lights.size() + 1);
			Light &light = lights[lights.size() - 1];
			light.pos = Vec3(0.0, 1.0, 0.0);
			light.radius = 2.5f;
			light.powerCorrection = 2.f;
			light.type = LightType_Spherical;
			light.material = Vec3(0., 0., 1.);
			light.isInCamSpace = false;
		}

		{ // Back Wall
			squares.resize(squares.size() + 1);
			Square &s = squares[squares.size() - 1];
			s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0., 0.), Vec3(0., 1., 0.), 2., 2.);
			s.scale(Vec3(2., 2., 1.));
			s.translate(Vec3(0., 0., -2.));
			s.build_arrays();
			s.material.diffuse_material = Vec3(0., 1., 1.);
			s.material.specular_material = Vec3(0., 1., 1.);
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
			s.material.diffuse_material = Vec3(1., 0., 0.);
			s.material.specular_material = Vec3(1., 0., 0.);
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
			s.material.diffuse_material = Vec3(0.0, 1.0, 0.0);
			s.material.specular_material = Vec3(0.0, 1.0, 0.0);
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
			s.material.diffuse_material = Vec3(1.0, 1.0, 1.0);
			s.material.specular_material = Vec3(1.0, 1.0, 1.0);
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
			s.material.diffuse_material = Vec3(1.0, 0.0, 1.0);
			s.material.specular_material = Vec3(1.0, 0.0, 1.0);
			s.material.shininess = 16;
		}

		/*
		{ // Front Wall
			squares.resize(squares.size() + 1);
			Square &s = squares[squares.size() - 1];
			s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0., 0.), Vec3(0., 1., 0.), 2., 2.);
			s.translate(Vec3(0., 0., -2.));
			s.scale(Vec3(2., 2., 1.));
			s.rotate_y(180);
			s.build_arrays();
			s.material.diffuse_material = Vec3(1.0, 1.0, 1.0);
			s.material.specular_material = Vec3(1.0, 1.0, 1.0);
			s.material.shininess = 16;
		}
		*/

		{ // GLASS Sphere

			spheres.resize(spheres.size() + 1);
			Sphere &s = spheres[spheres.size() - 1];
			s.m_center = Vec3(1.0, -1.25, 0.5);
			s.m_radius = 0.75f;
			s.build_arrays();
			s.material.type = Material_Mirror;
			s.material.diffuse_material = Vec3(1., 0., 0.);
			s.material.specular_material = Vec3(1., 0., 0.);
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
			s.material.diffuse_material = Vec3(1., 1., 0.);
			s.material.specular_material = Vec3(1., 1., 0.);
			s.material.shininess = 16;
			s.material.transparency = 0.;
			s.material.index_medium = 0.;
		}

	}

};

#endif
