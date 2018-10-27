import React from "react";

import { view } from "./app";
import { ApiClient } from "./api";

const pushModel = () => {
  console.log("Pushing model to server");
  const xml = view.exportModel();
  const meta = view.exportMeta();
  console.log("POSTing model");
  ApiClient.post("/model", { model: xml });
  console.log("POSTing meta:", meta);
  ApiClient.post("/model/meta", meta);
  console.log("Done!s");
};

export const ExportButton = () => {
  return (
    <button
      onClick={() => {
        pushModel();
      }}
    >
      Push to app
    </button>
  );
};
