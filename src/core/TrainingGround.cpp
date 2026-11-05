#include "core/TrainingGround.h"
#include <glm/vec2.hpp>

namespace craft::core {

TrainingGround makeTrainingGround(int) {
  TrainingGround ground{"COURTYARD / Dusk exercise", {16, 2.6F, 26.5F}, -90, {}, {}, {}};
  const auto fill = [&](int x0, int x1, int y0, int y1, int z0, int z1,
                         GroundMaterial material) {
    for (int x = x0; x <= x1; ++x)
      for (int y = y0; y <= y1; ++y)
        for (int z = z0; z <= z1; ++z)
          ground.blocks.push_back({{x, y, z}, material});
  };
  fill(2, 29, 0, 0, 2, 29, GroundMaterial::Concrete);
  fill(2, 29, 1, 5, 2, 5, GroundMaterial::Wall);
  fill(2, 4, 1, 4, 10, 17, GroundMaterial::Wall);
  fill(27, 29, 1, 4, 6, 13, GroundMaterial::Wall);
  fill(2, 2, 1, 2, 6, 28, GroundMaterial::Cover);
  fill(29, 29, 1, 2, 14, 28, GroundMaterial::Cover);
  fill(2, 13, 1, 2, 29, 29, GroundMaterial::Cover);
  fill(19, 29, 1, 2, 29, 29, GroundMaterial::Cover);
  fill(14, 18, 4, 4, 29, 29, GroundMaterial::Cover);
  for (int x : {5, 12, 22}) {
    fill(x, x + 3, 3, 3, 5, 5, GroundMaterial::Blue);
    fill(x, x + 3, 4, 4, 5, 5, GroundMaterial::Cover);
  }
  fill(2, 29, 5, 5, 5, 5, GroundMaterial::Cover);
  for (const glm::ivec2 p : {glm::ivec2(10, 11), {21, 11}, {10, 20}, {22, 20}}) {
    fill(p.x, p.x + 1, 1, 4, p.y, p.y + 1, GroundMaterial::Concrete);
    fill(p.x, p.x + 1, 1, 1, p.y, p.y + 1, GroundMaterial::Yellow);
  }
  fill(14, 18, 0, 0, 24, 24, GroundMaterial::Yellow);
  fill(5, 26, 0, 0, 6, 6, GroundMaterial::Yellow);
  fill(5, 7, 1, 1, 20, 21, GroundMaterial::Cover);
  fill(25, 27, 1, 1, 16, 17, GroundMaterial::Cover);
  ground.targets = {{{16, 1, 8.5F}, false}, {{7.5F, 1, 14.5F}, false},
                     {{25.5F, 1, 12.5F}, false}, {{6, 1, 8.5F}, true},
                     {{25.5F, 1, 23.5F}, true}};
  ground.lights = {{{5, 8, 6.5F}, {13, 1, 15}}, {{26, 8, 6.5F}, {20, 1, 15}},
                    {{5, 8, 27}, {13, 1, 20}}, {{27, 8, 27}, {20, 1, 20}}};
  return ground;
}

}
