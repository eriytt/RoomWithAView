import * as THREE from "three";

import { ThreeCanvasComponent } from "./ThreeCanvasComponent";
import { ColorMenuComponent } from "./ColorMenu";
//import { OgreMaxLoader } from "./OgreMaxLoader";
import { OgreLoader } from "./OgreLoader";
import { ApiClient } from "./api";

export type MeshObject = {
  mesh: THREE.Mesh;
  name: string;
};

export class View {
  state: {
    objects: Array<MeshObject>;
    currentObject: MeshObject | undefined;
    shadeObject: MeshObject | undefined;
  } = {
    objects: [],
    currentObject: undefined,
    shadeObject: undefined
  };

  mycanvas: ThreeCanvasComponent | undefined = undefined;

  pickObject(cmp: ThreeCanvasComponent, hits: Array<THREE.Intersection>) {
    console.log("Picking objects:", hits);

    if (hits.length == 0) {
      if (this.state.shadeObject) cmp.scene.remove(this.state.shadeObject.mesh);
      this.state.currentObject = undefined;
      return;
    }

    const oldCurrent = this.state.currentObject;
    this.state.currentObject = this.state.objects.find(o => {
      return o.mesh === hits[0].object;
    });

    if (this.state.currentObject !== oldCurrent) {
      console.log("Object changed");
      if (this.state.shadeObject) cmp.scene.remove(this.state.shadeObject.mesh);
      if (this.state.currentObject) {
        const g = this.state.currentObject.mesh
          .geometry as THREE.BufferGeometry;
        const c_geom = new THREE.BufferGeometry();

        c_geom.addAttribute("position", g.getAttribute("position"));

        const index = g.index;
        if (index != null) c_geom.setIndex(index);

        const c_mat = new THREE.MeshBasicMaterial({
          color: 0x0000ff,
          opacity: 0.3,
          transparent: true
        });

        const c_mesh = new THREE.Mesh(c_geom, c_mat);
        cmp.scene.add(c_mesh);
        this.state.shadeObject = { mesh: c_mesh, name: "Shade" };
      }
    }
    /* for (var i = 0; i < hits.length; i++) {
     *   console.log("Intersecting:", hits[i]);
     *   const o = hits[i].object;
     *   if (o.type === "Mesh") {
     *     const mo = o as THREE.Mesh;
     *     (mo.material as THREE.MeshBasicMaterial).color.setHex(0xff0000);
     *   }
     * }*/
  }

  setObjectColor(color: string) {
    if (this.state.currentObject === undefined) return;

    const m = this.state.currentObject.mesh as THREE.Mesh;
    const mat = m.material as THREE.MeshBasicMaterial;
    mat.color.setHex(parseInt(color.slice(1), 16));
    this.mycanvas!.updateCanvas();
  }

  setObjectTexture(img: string) {
    if (this.state.currentObject === undefined) return;

    const m = this.state.currentObject.mesh as THREE.Mesh;
    const mat = m.material as THREE.MeshBasicMaterial;

    console.log("Loading texture", img);
    const texture = new THREE.TextureLoader().load(img, () => {
      console.log("Texture loaded", texture);
      mat.map = texture;
      this.mycanvas!.updateCanvas();
    });
    texture.wrapS = THREE.RepeatWrapping;
    texture.wrapT = THREE.RepeatWrapping;
    const repeat = 2;
    texture.repeat.set(repeat, repeat);

    this.mycanvas!.updateCanvas();
  }

  setupScene(cmp: ThreeCanvasComponent) {
    this.mycanvas = cmp;

    const metaP = ApiClient.get("/model/meta")
      .then(response => {
        return response.json();
      })
      .catch(e => {
        console.log(`There was a problem loading your model: ${e}`);
      });

    const ogreloader = new OgreLoader();
    ogreloader.load("modelxml").then(async meshes => {
      console.log(`Model modelxml loaded: ${meshes.length} meshes`);

      const json = await metaP;

      meshes.forEach(m => {
        const meshMeta = json[m.name];
        if (meshMeta) {
          const materialType = meshMeta["type"];
          if (materialType == "SolidColor") {
            const material = new THREE.MeshBasicMaterial({
              color: parseInt(meshMeta["color"], 16)
            });
            m.material = material;
          } else if (materialType == "Texture") {
            const texture = new THREE.TextureLoader().load(
              meshMeta["uri"],
              () => {
                console.log("Texture loaded");
                this.mycanvas!.updateCanvas();
              },
              () => {
                console.log("Texture load in progress");
              },
              err => {
                console.error("Error loading texture:", err);
              }
            );
            texture.wrapS = THREE.RepeatWrapping;
            texture.wrapT = THREE.RepeatWrapping;
            const repeat = 2;
            texture.repeat.set(repeat, repeat);

            const material = new THREE.MeshBasicMaterial({
              map: texture
            });
            m.material = material;
          }
        }

        this.state.objects.push({ mesh: m, name: m.name });
        cmp.scene.add(m);
      });
    });

    cmp.picker = (cmp, hits) => {
      this.pickObject(cmp, hits);
    };
  }

  exportMeta() {
    let meta: any = {};

    const objects = this.state.objects;
    objects.forEach(mo => {
      const m = mo.mesh as THREE.Mesh;
      const mat = m.material as THREE.MeshBasicMaterial;

      if (mat.map !== null) {
        meta[mo.name] = {
          type: "Texture",
          uri: new URL(mat.map.image.currentSrc).pathname
        };
      } else {
        meta[mo.name] = { type: "SolidColor", color: mat.color.getHexString() };
      }
    });
    return meta;
  }

  exportModel() {
    console.log(this.state.objects[4]);
    const submeshnames: Array<string> = [];
    const xml = ['<?xml version="1.0" encoding="UTF-8"?>'];
    xml.push("<mesh>");

    const objects = this.state.objects;
    xml.push("<submeshes>");
    objects.forEach((mo, index) => {
      submeshnames.push(`<submeshname index="${index}" name="${mo.name}"/>`);
      const sm = mo.mesh;
      const geom = sm.geometry as THREE.BufferGeometry;
      const coords = geom.getAttribute("position");
      const uvs = geom.getAttribute("uv");

      xml.push(
        '<submesh operationtype="triangle_list" use32bitindexes="False" usesharedvertices="False">'
      );
      xml.push(`<geometry vertexcount="${coords.count}">`);
      xml.push(
        `<vertexbuffer colours_diffuse="False" normals="false" positions="true" texture_coords="${
          uvs ? 1 : 0
        }">`
      );

      for (let i = 0; i < coords.count; ++i) {
        const startIdx = i * coords.itemSize;
        const v1 = coords.array[startIdx];
        const v2 = coords.array[startIdx + 1];
        const v3 = coords.array[startIdx + 2];
        xml.push("<vertex>");
        xml.push(`<position x="${v1}" y="${v2}" z="${v3}"/>`);
        if (uvs) {
          const startIdx = i * uvs.itemSize;
          const u = uvs.array[startIdx];
          const v = uvs.array[startIdx + 1];
          xml.push(`<texcoord u="${u}" v="${v}"/>`);
        }
        xml.push("</vertex>");
      }
      xml.push("</vertexbuffer>");
      xml.push("</geometry>");

      xml.push(`<faces count="${coords.count / coords.itemSize}">`);

      for (let i = 0; i < coords.count / coords.itemSize; ++i) {
        xml.push(`<face v1="${i * 3}" v2="${i * 3 + 1}" v3="${i * 3 + 2}" />`);
      }
      xml.push("</faces>");

      xml.push("</submesh>");
    });

    xml.push("</submeshes>");

    xml.push("<submeshnames>");
    xml.push(...submeshnames);
    xml.push("</submeshnames>");

    xml.push("</mesh>");

    const xmlString = xml.join("\n");
    return xmlString;
  }
}
