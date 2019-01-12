import React from "react";
import { render } from "react-dom";
import { BrowserRouter } from "react-router-dom";

import { ThreeCanvasComponent } from "./ThreeCanvasComponent";
import { ColorMenuComponent } from "./ColorMenu";
import { FloorMenuComponent } from "./FloorMenu";
import { ExportButton } from "./ExportButtonComponent";

import { View } from "./view";

export const view = new View();
const width = 1000;
const height = 700;

const App = () => {
  return (
    <div
      style={{
        display: "grid",
        gridTemplateColumns: "70% 30%",
        gridTemplateRows: "1fr 1fr"
      }}
    >
      <div>
        <ThreeCanvasComponent
          width={width}
          height={height}
          setup={cmp => {
            view.setupScene(cmp);
          }}
        />
      </div>
      <div>
        <ColorMenuComponent />
        <FloorMenuComponent />
        <ExportButton />
      </div>
    </div>
  );
};

const Router = () => {
  return (
    <BrowserRouter>
      <App />
    </BrowserRouter>
  );
};

render(<Router />, document.getElementById("root"));
