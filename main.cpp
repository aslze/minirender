#include "Scene.h"
#include "renderer.h"
#include "io.h"
#include <asl/CmdArgs.h>

using namespace asl;

int main(int argc, char* argv[])
{
	CmdArgs args;

	double t1 = now();

	TriMesh* shape = loadSTL(args[0]);

	double t2 = now();

	printf("load %s %.3f s\n", *args[0], t2-t1);

	SWRenderer renderer;

	renderer.setProjection(projectionFrustum(0.75f, renderer.aspect(), 40, 1000));
	renderer.setView(Matrix4::translate(0, 0, -100));
	renderer.setLight(Vec3(-0.15f, 0.6f, 1));
	renderer.setScene(shape);

	int n = 100;

	for (float i = 0; i < n; i += 1)
	{
		double t3 = now();
		renderer.clear();
		renderer.setView(Matrix4::translate(0, 0, -100) * Matrix4::rotateY(3.8f + 0.1f * i) * Matrix4::rotateZ(0.1f) * Matrix4::rotateX(-0.35f));
		renderer.paintMesh(shape, Matrix4::rotateY(i*0.1f));

		savePPM(renderer.getImage(), String::f("out%04i.ppm", (int)i));

		double t5 = now();
	}

	double t6 = now();
	printf("t=%f (t frame = %f)\n", t6-t2, (t6-t2)/n);
	return 0;
}
