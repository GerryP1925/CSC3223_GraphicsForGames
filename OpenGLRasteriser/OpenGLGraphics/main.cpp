#include "Renderer.h"
#include "RenderObject.h"

#pragma comment(lib, "nclgl.lib")

#define NUM_SHADERS 8
Shader *g_shaders[NUM_SHADERS];

void delete_shaders()
{
  for (int i = 0; i < NUM_SHADERS; i++)
  {
    if (g_shaders[i] != NULL)
      delete g_shaders[i];

    g_shaders[i] = NULL;
  }
}

void load_shaders()
{
  for (int i = 0; i < NUM_SHADERS; i++)
    g_shaders[i] = NULL;

  g_shaders[0] = new Shader("basic_vertex.glsl", "basic_fragment.glsl");
  g_shaders[1] = new Shader("shrink_vertex.glsl", "basic_fragment.glsl");
  g_shaders[2] = new Shader("basic_vertex.glsl", "texfade_fragment.glsl");
  g_shaders[3] = new Shader("basic_vertex.glsl", "fade_fragment.glsl");
  g_shaders[4] = new Shader("nomvp_vertex.glsl", "basic_fragment.glsl", "split_geometry.glsl");
  g_shaders[5] = new Shader("nomvp_vertex.glsl", "basic_fragment.glsl", "", "detail_tcs.glsl",
                            "detail_tes.glsl");
  g_shaders[6] = new Shader("lighting_vertex.glsl", "lighting_fragment.glsl");
  g_shaders[7] = new Shader("nomvp_vertex.glsl", "lighting_fragment.glsl", "", "detail_tcs.glsl",
                            "detail_tes.glsl");

  int failures = 0;
  for (int i = 0; i < NUM_SHADERS; i++)
  {
    if (g_shaders[i] != NULL && g_shaders[i]->UsingDefaultShader())
    {
      failures++;
      cout << "Shader " << i << " failed to load or compile." << endl;
    }
  }

  if (failures == 0)
    cout << "All shaders loaded and compiled successfully" << endl;
}

void main(void)
{
  Window w = Window(800, 600);
  Renderer r(w);

  // Load the cube mesh and textures
  Mesh *cubeMesh = Mesh::LoadMeshFile("cube.asciimesh");
  GLuint cubeNormalTexture = r.LoadTexture("bricks.png");
  GLuint cubeDestroyedTexture = r.LoadTexture("bricks_destroyed.png");
  GLuint cubeHeightmap = r.LoadTexture("bricks_heightmap.png");

  // Create cube render object
  RenderObject cube(cubeMesh, g_shaders[0], cubeNormalTexture);
  cube.SetTexture(1, cubeDestroyedTexture);
  cube.SetTexture(2, cubeHeightmap);

  // Create lasers (one in a fixed position, one a child of the cube)
  Mesh *staticLaserMesh = Mesh::GenerateLine(Vector3(-2.0, 0.0, 10.0), Vector3(0.0, 0.0, -10.0));
  Mesh *movingLaserMesh = Mesh::GenerateLine(Vector3(0.0, 0.0, 0.0), Vector3(0.0, 0.0, 10.0));
  Shader *laserShader = new Shader("basic_vertex.glsl", "notex_fragment.glsl");
  RenderObject staticLaser(staticLaserMesh, laserShader);
  RenderObject movingLaser(movingLaserMesh, laserShader);
  cube.AddChild(movingLaser);

  // Load and compile the shaders
  load_shaders();

  cube.SetModelMatrix(Matrix4::Translation(Vector3(0.0, 0.0, -10.0)));
  staticLaser.SetModelMatrix(Matrix4::Translation(Vector3(0.0, 1000.0, 0.0)));
  movingLaser.SetModelMatrix(Matrix4::Translation(Vector3(0.0, 1000.0, 0.0)));
  r.AddRenderObject(cube);
  r.AddRenderObject(staticLaser);

  r.SetProjectionMatrix(Matrix4::Perspective(1, 100, 1.33f, 45.0f));
  r.SetViewMatrix(Matrix4::BuildViewMatrix(Vector3(0, 0, 0), Vector3(0, 0, -10)));

  r.SetLighting(0, Vector3(5.0f, 10.0f, 10.0f), 1000.0f, Vector3(1, 1, 1));
  // Initiallty set second light to not affect scene
  r.SetLighting(1, Vector3(0.0f, 1000.0f, 0.0f), 0.0f, Vector3(0, 0, 0));

  cube.SetShader(g_shaders[0]);

  // Print the list of key brindings for shader demos
  cout << endl
       << "Key bindings:" << endl
       << "r - Reset scene (reset before changing demo mode)" << endl
       << "p - Pause animation" << endl
       << "P - Pause rotation" << endl
       << "0 - Reload and compile shaders" << endl
       << "+ - Zoom in" << endl
       << "- - Zoom out" << endl
       << "s - Shrink the cube until it disappears" << endl
       << "d - Fades form the normal texture to a destroyed texture" << endl
       << "f - Fade the cube to transaparent" << endl
       << "a - Split the cube into several smaller cubes" << endl
       << "h - Add heightmap" << endl
       << "H - Add heightmap with lighting" << endl
       << "l - Static laser with lighting" << endl
       << "L - Moving laser with lighting" << endl;

  bool rotate = true;
  bool disableDepthDuringAnim = false;

  while (w.UpdateWindow())
  {
    float msec = w.GetTimer()->GetTimedMS();

    // Pause
    if (Keyboard::KeyTriggered(KEY_P))
    {
      if (Keyboard::KeyDown(KEY_SHIFT))
        rotate = !rotate;
      else
        r.animPause();
    }

    // Reload shaders
    if (Keyboard::KeyTriggered(KEY_0))
    {
      delete_shaders();
      load_shaders();
      cube.SetShader(g_shaders[0]);
    }

    // Reset scene
    if (Keyboard::KeyTriggered(KEY_R))
    {
      // Reset default values
      r.SetLighting(1, Vector3(0.0f, 1000.0f, 0.0f), 0.0f, Vector3(0, 0, 0));
      staticLaser.SetModelMatrix(Matrix4::Translation(Vector3(0.0, 1000.0, 0.0)));
      movingLaser.SetModelMatrix(Matrix4::Translation(Vector3(0.0, 1000.0, 0.0)));
      cubeMesh->type = GL_TRIANGLES;
      glEnable(GL_DEPTH_TEST);

      r.animStop();
      cube.SetShader(g_shaders[0]);
    }

    // Zoom in
    if (Keyboard::KeyTriggered(KEY_PLUS))
    {
      cube.SetModelMatrix(Matrix4::Translation(Vector3(0.0f, 0.0f, 0.5f)) * cube.GetModelMatrix());
    }

    // Zoom out
    if (Keyboard::KeyTriggered(KEY_MINUS))
    {
      cube.SetModelMatrix(Matrix4::Translation(Vector3(0.0f, 0.0f, -0.5f)) * cube.GetModelMatrix());
    }

    // Shrink cube
    if (Keyboard::KeyTriggered(KEY_S))
    {
      cube.SetShader(g_shaders[1]);
      r.animStart();
    }

    // Fade to destroyed texture
    if (Keyboard::KeyTriggered(KEY_D))
    {
      cube.SetShader(g_shaders[2]);
      r.animStart();
    }

    // Fade cube to transparent
    if (Keyboard::KeyTriggered(KEY_F))
    {
      disableDepthDuringAnim = true;
      cube.SetShader(g_shaders[3]);
      r.animStart();
    }

    // Split the cube into several smaller cubes
    if (Keyboard::KeyTriggered(KEY_A))
    {
      cube.SetShader(g_shaders[4]);
      r.animStart();
    }

    // Add heightmap
    if (Keyboard::KeyTriggered(KEY_H))
    {
      if (Keyboard::KeyHeld(KEY_SHIFT))
      {
        // With lighting
        cubeMesh->type = GL_PATCHES;
        cube.SetShader(g_shaders[7]);
      }
      else
      {
        // No lighting
        cubeMesh->type = GL_PATCHES;
        cube.SetShader(g_shaders[5]);
      }
    }

    // Laser
    if (Keyboard::KeyTriggered(KEY_L))
    {
      cube.SetShader(g_shaders[6]);
      if (Keyboard::KeyHeld(KEY_SHIFT))
      {
        // Move the moving laser back into the scene
        movingLaser.SetModelMatrix(Matrix4::Translation(Vector3(0.0, 0.0, 0.0)));
      }
      else
      {
        // Move the static laser back into the scene
        staticLaser.SetModelMatrix(Matrix4::Translation(Vector3(0.0, 0.0, 0.0)));
        // Add the red light source
        r.SetLighting(1, Vector3(-2.0f, 0.0f, 10.0f), 50.0f, Vector3(1, 0, 0));
      }
    }

    // Disable depth test part way through an animation, useful for fading to transparency
    if (disableDepthDuringAnim && glIsEnabled(GL_DEPTH_TEST) && r.getAnimPosition() > 0.25)
    {
      glDisable(GL_DEPTH_TEST);
      disableDepthDuringAnim = false;
    }

    // Rotate cube
    if (rotate)
      cube.SetModelMatrix(cube.GetModelMatrix() * Matrix4::Rotation(0.1f * msec, Vector3(0, 1, 1)));

    r.UpdateScene(msec);
    r.ClearBuffers();
    r.RenderScene();
    r.SwapBuffers();
  }
}
