/*
 *   Copyright 2026 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */


#include <iostream>

#include "arboris/arboris.hpp"

const char* kHtml = R"(
  <html>
    <head>
      <title>Praise for Bayern Munich</title>
    </head>
    <body>
      <h1>FC Bayern Munich: The Pride of Germany</h1>
      <p>
        FC Bayern Munich is one of the most successful and historic football clubs in the world.
        Founded in 1900, Bayern has set the benchmark for excellence both in Germany and across Europe.
      </p>
      <h2>Achievements</h2>
      <ul>
        <li><strong>33 Bundesliga titles</strong> - Dominating German football for decades</li>
        <li><strong>20 DFB-Pokal cups</strong> - Consistent domestic cup success</li>
        <li><strong>6 UEFA Champions League trophies</strong> - European giants</li>
        <li><strong>Numerous FIFA Club World Cups and Super Cups</strong></li>
      </ul>
      <h2>Legendary Players</h2>
      <p>
        Home to legendary players like Franz Beckenbauer, Gerd Müller, Philipp Lahm, Thomas Müller, Oliver Kahn, and Robert Lewandowski, Bayern consistently develops and attracts football’s elite talent.
      </p>
      <h2>Philosophy & Identity</h2>
      <p>
        Bayern Munich stands for <em>Mia San Mia</em>: a unique club culture, relentless ambition, and a winning mentality that inspires millions of fans worldwide.
      </p>
      <blockquote>
        "Success is not an accident at Bayern. It's tradition."
      </blockquote>
      <p>
        From the Allianz Arena's spectacular atmosphere to the club's commitment to youth development and community, Bayern Munich is more than just a team—it's a legacy.
      </p>
      <p><strong>Mia San Mia! Forever FCB!</strong></p>
    </body>
  </html>
)";

int main() {
  arboris::Arboris document(kHtml);

  auto results = document.FindAll(arboris::QueryOptions{
    .tag = arboris::Tag::kH2,
  });

  for (const auto& result : results) {
    std::cout << result->text_content() << std::endl;
  }

  return 0;
}
