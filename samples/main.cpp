#include <minirender/Scene.h>
#include <minirender/Renderer.h>
#include <minirender/io.h>
#include <asl/CmdArgs.h>
#include <asl/Console.h>

using namespace asl;
using namespace minirender;


inline String console_bg() { return "\033[48;2;"; }

inline String console_fg() { return "\033[38;2;"; }

inline String console_rgb(int r, int g, int b)
{
	return String(15, "%i;%i;%im", clamp(r, 0, 255), clamp(g, 0, 255), clamp(b, 0, 255));
}

// paint image to console, printing ' ' with different background colors

void consolePaint(const asl::Array2<asl::Vec3>& image)
{
	console.clear();
	for (int i = 0; i < image.rows(); i++)
	{
		String line;
		for (int j = 0; j < image.cols(); j++)
		{
			auto c = (image(i, j) * 255).with<int>();

			line << console_bg() << console_rgb(c.x, c.y, c.z) << ' ';
		}
		printf("%s\n", *line);
	}
}


int main(int argc, char* argv[])
{
	CmdArgs args;

	bool useconsole = args.has("console");
	int n = args["n"] | 1;
	float d = args["d"] | 170;
	bool save = args.has("save");
	int sizew = args["w"] | 800;
	int sizeh = args["h"] | (sizew * 3 / 4);
	float wx = deg2rad(float(args["rx"] | 0));

	double t1 = now();

	auto shape = loadMesh(args[0]);

	if (!shape)
		return 1;

	double t2 = now();

	printf("load %s %.3f s\n", *args[0], t2 - t1);

	Scene* scene = new Scene();
	
	scene->children << shape;
	scene->ambientLight = 0.2f;

	Renderer renderer;

	renderer.setProjection(projectionFrustum(deg2rad(35.0f), renderer.aspect(), 10, 7000));
	renderer.setView(Matrix4::translate(0, 0, -100));
	renderer.setLight(Vec3(-0.15f, 0.6f, 1));
	renderer.setScene(scene);
	renderer.setSize(sizew, sizeh);

	Array<double> times;

	for (float i = 0; i < n; i += 1)
	{
		renderer.setView(Matrix4::translate(0, -2.6f, -d) * Matrix4::rotateX(deg2rad(-60.0f)));
		
		shape->transform = Matrix4::rotateZ(i * 0.05f) * Matrix4::rotateX(-i * wx);

		if (useconsole)
		{
			auto s = console.size();
			renderer.setSize(s.w, s.h);
		}

		renderer.render();

		double t1 = now();

		if (useconsole)
			consolePaint(renderer.getImage());

		if (save)
			savePPM(renderer.getImage(), String::f("out%04i.ppm", (int)i));

		double t2 = now();
		times << t2 - t1;
	}

	double t6 = now();
	printf("t=%f (t frame = %f)\n", t6-t2, (t6-t2)/n);

	double tp = 0;
	for (auto t : times)
		tp += t;
	printf("t paint = %f\n", tp / times.length());
	return 0;
}
