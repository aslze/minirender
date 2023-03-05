#include <minirender/Scene.h>
#include <minirender/Renderer.h>
#include <minirender/io.h>
#include <asl/CmdArgs.h>

using namespace asl;
using namespace minirender;

float rf(float z, float s)
{
	return 30 + 40 * sin(4 * z * (float)PI / s) / (4 * z * (float)PI / s);
}

TriMesh* createObject(int m, int n, bool usetex, Random& random)
{
	TriMesh* mesh = new TriMesh();

	float da = 2 * (float)PI / n;
	float dz = 70.f / m;

	for (int i = 0; i < m; i++)
	{
		float z = i * dz;
		float r = rf(z, 100);
		Vec2 nor = Vec2(1, -(rf(z + 0.001f, 100) - rf(z, 100)) / 0.001f).normalized();
		for (int j = 0; j < n; j++)
		{
			Vec3 p(r * cos(j * da), r * sin(j * da), z);
			mesh->vertices << p;
			mesh->normals << Vec3(nor.x * cos(j * da), nor.x * sin(j * da), nor.y).normalized();

			if (usetex)
				mesh->texcoords << Vec2((float)j / n, (float)i / m);

			if (j > 0 && i > 0)
				mesh->indices << (n * (i - 1) + j - 1) << (n * (i - 1) + j) << (n * i + j) << (n * (i - 1) + j - 1)
				              << (n * i + j) << (n * i + j - 1);
		}
	}
	
	mesh->normalsI = mesh->indices;
	if (usetex)
		mesh->texcoordsI = mesh->indices;

	mesh->material = new Material();
	mesh->material->shininess = 15;

	if (usetex) {
		Vec3 color = { random(1.f), random(1.f), random(1.f) };
		Array2<Vec3> tex(256, 256);
		for (int i = 0; i < tex.rows(); i++)
			for (int j = 0; j < tex.cols(); j++)
				tex(i, j) = color * (0.75f + 0.25f * (cos(i * 40 / 256.f) * sin(j * 40 / 256.f)));
		mesh->material->texture = tex;
	}

	return mesh;
}


int main(int argc, char** argv)
{
	CmdArgs args(argc, argv);

	float d = args["d"] | 700;                  // camera distance
	int n = args["n"] | 10;                     // number of frames
	int sizew = args["w"] | 1920;               // image size (not for console)
	int sizeh = args["h"] | (sizew * 9 / 16);
	float wx = deg2rad(float(args["rx"] | 0));  // angular X speed deg/s
	float wz = deg2rad(float(args["rz"] | 40)); // angular Z speed deg/s
	float fov = deg2rad(35.f);
	float yaw = deg2rad(float(args["yaw"] | 0));
	float tilt = deg2rad(float(args["tilt"] | 20));
	bool  usetex = args.has("tex");
	bool  nolight = args.has("dark");

	bool saving = args.has("save");

	Scene* scene = new Scene();

	Random random(false);

	for (int i = 0; i < 20; i++)
	{
		auto shape = createObject(200, 200, usetex, random);

		shape->material->diffuse = { random(1.f), random(1.f) ,random(1.f) };
		shape->transform = Matrix4::translate(random(-180.f, 180.f), random(-180.f, 180.f), random(-100.f, 100.f)) * Matrix4::rotate({ random(1.f), random(1.f), random(1.f) });
		scene->children << shape;
	}
	scene->ambientLight = 0.2f;

	auto box = scene->getBbox();

	auto size = box.size();
	auto center = box.center();

	Renderer renderer;

	renderer.setLight(Vec3(-0.4f, .6f, 1.f));
	//enderer.setLight({ 0, 0, 0 }, true);
	renderer.setScene(scene);
	renderer.setSize(sizew, sizeh);
	renderer.setProjection(projectionFrustum(fov, renderer.aspect(), 10, 7000));
	renderer.setLighting(!nolight);

	Array<double> times;

	float rx = -(float)PI / 2 + tilt, rz = yaw;

	double t2 = now();

	double t0 = now();

	for (float i = 0; i < n; i++)
	{
		double t = now();
		float dt = 0.1f;
		t0 = t;

		rz += wz * dt;
		rx += wx * dt;

		renderer.setView(Matrix4::translate(0, 0, -d) * Matrix4::rotateX(rx) * Matrix4::rotateZ(rz));

		renderer.render();

		if (saving)
			savePPM(renderer.getImage(), String::f("bench%04i.ppm", (int)i));

		double ta = now();

		times << now() - ta;
	}

	double t6 = now();

	printf("t = %.3f (t frame = %.3f)\n", t6 - t2, (t6 - t2) / n);

	return 0;
}
