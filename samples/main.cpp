#include <minirender/Scene.h>
#include <minirender/Renderer.h>
#include <minirender/io.h>
#include <asl/CmdArgs.h>
#include <asl/Console.h>

using namespace asl;
using namespace minirender;


// paint image to console, printing ' ' with different background colors

void consolePaint(const asl::Array2<asl::Vec3>& image)
{
	console.gotoxy(0, 0);
	String line;
	for (int i = 0; i < image.rows(); i++)
	{
		line = "";
		for (int j = 0; j < image.cols(); j++)
		{
			auto c = (image(i, j) * 255).with<int>();

			line << console.bg() << console.rgb(c.x, c.y, c.z) << ' ';
		}
		printf("%s\n", *line);
	}
}


int main(int argc, char* argv[])
{
	CmdArgs args(argc, argv);

	bool useconsole = args.has("console");      // if given outputs to console
	bool saving = args.has("save");             // if given saves images as PPM
	float d = args["d"] | 140;                  // camera distance
	int n = args["n"] | 1;                      // number of frames
	double tout = args["t"] | 0;                // timeout in seconds (0 -> just n frames)
	int sizew = args["w"] | 800;                // image size (not for console)
	int sizeh = args["h"] | (sizew * 3 / 4);
	float wx = deg2rad(float(args["rx"] | 0));  // angular X speed deg/s
	float wz = deg2rad(float(args["rz"] | 40)); // angular Z speed deg/s
	float par = 0.5f;                           // pixel aspect ratio in console (chars not square)
	bool oldconsole = args.has("oldconsole");   // console suports only 256 colors (there are even older ones)
	bool fit = args.has("fit") || !args.has("d");
	float fov = deg2rad(35.f);
	float yaw = deg2rad(float(args["yaw"] | 0));
	float tilt = deg2rad(float(args["tilt"] | 20));
	Vec3 bg = args["bgcolor"].split(',').resize(3).with<float>().ptr();

	bg /= 255;
	
	String outname = args["o"] | ((n == 1) ? "out.ppm" : "out%04i.ppm");

	if (args.length() == 0)
	{
		printf("Render one or many images of a 3D model (STL or OBJ) to the console or to PPM files\n\n"
			"render [options] file\n"
			" -o <string> file name to save image(s) to (use -- for stdout, or \"...%%04i...\" for many frames)\n"
			" -n <int> number of frames to paint (default: 1)\n"
			" -t <float> timeout in seconds for 'animation'\n"
			" -w <int> image horizontal size (default: 800)\n"
			" -h <int> image vertical size (default: w * 3/4)\n"
			" -yaw <float> rotation angle around Z (vertical axis) in degrees\n"
			" -tilt <float> rotation angle around X axis in degrees\n"
			" -rx <float> angular speed around X in degrees/s\n"
			" -rz <float> angular speed around Z in degrees/s\n"
			" -d <float> camera distance to origin (default: automatic to fit scene)\n"
			" -bgcolor <int,int,int> RGB color of background\n\n"
			" -console! render to the console\n"
			" -oldconsole! support old consoles with only 256 colors\n"
		);
		return 0;
	}


	if (args.has("o"))
		saving = true;

	bool silent = (outname == "--");

	if (useconsole && tout > 0)
		n = 1000000;
	else
		tout = 1e10;

	double t1 = now();

	auto shape = loadMesh(args[0]);

	if (!shape)
		return 1;

	double t2 = now();

	if (!silent)
		printf("load %s %.3f s\n", *args[0], t2 - t1);

	Scene* scene = new Scene();

	scene->children << shape;
	scene->ambientLight = 0.2f;

	auto box = scene->getBbox();

	auto size = box.size();
	auto center = box.center();

	if (fit)
	{
		float dh = max(size.x, size.y) / (2 * tan(fov * sizew / sizeh / 2));
		float dv = size.z / (2 * tan(fov / 2));
		d = 1.55f * max(dh, dv);
		scene->transform = Matrix4::translate(-center);
	}


	Renderer renderer;

	renderer.setBackground(bg);
	renderer.setLight(Vec3(-0.3f, 0.55f, 1));
	renderer.setScene(scene);
	renderer.setSize(sizew, sizeh);
	renderer.setProjection(projectionFrustum(fov, renderer.aspect(), 10, 7000));

	Array<double> times;

	float rx = -(float)PI/2 + tilt, rz = yaw;

	if (oldconsole)
		console.setColorMode(1);

	double t0 = now();

	for (float i = 0; i < n; i++)
	{
		double t = now();
		float dt = useconsole ? float(t - t0) : 0.1f;
		t0 = t;

		if (t - t2 > tout)
		{
			break;
		}

		rz += wz * dt;
		rx += wx * dt;

		renderer.setView(Matrix4::translate(0, 0, -d) * Matrix4::rotateX(rx) * Matrix4::rotateZ(rz));

		if (useconsole)
		{
			static Console::Size cs;
			auto s = console.size(); // set size and aspect based on current console size
			if (s.w != cs.w || s.h != cs.h)
			{
				console.clear();
				cs = s;
			}
			renderer.setSize(s.w, s.h - 1);
			renderer.setProjection(projectionFrustum(fov, par * renderer.aspect(), 10, 7000));
		}

		renderer.render();

		double ta = now();

		if (useconsole)
			consolePaint(renderer.getImage());

		if (saving)
			savePPM(renderer.getImage(), n == 1 ? outname : String::f(*outname, (int)i));

		times << now() - ta;
	}

	double t6 = now();

	double tp = 0;  // average time between frames
	for (auto t : times)
		tp += t;

	if (!silent)
	{
		printf("t = %.3f (t frame = %.3f)\n", t6 - t2, (t6 - t2) / n);
		printf("t paint = %.3f\n", tp / times.length());
	}
	return 0;
}
