import * as THREE from "three";

import { ThreeCanvasComponent } from "./ThreeCanvasComponent";
import { ColorMenuComponent } from "./ColorMenu";
//import { OgreMaxLoader } from "./OgreMaxLoader";
import { OgreLoader } from "./OgreLoader";

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
        const c_geom = new THREE.BufferGeometry();
        c_geom.addAttribute(
          "position",
          (this.state.currentObject.mesh
            .geometry as THREE.BufferGeometry).getAttribute("position")
        );
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

    // prettier-ignore
    const wall1 = new Float32Array( [
        1.0, -1.0,  1.0,
        -1.0, -1.0,  1.0,
        1.0,  1.0,  1.0,

        -1.0,  1.0,  1.0,
        1.0,  1.0,  1.0,
        -1.0, -1.0,  1.0
    ] );

    const w1_geom = new THREE.BufferGeometry();
    w1_geom.addAttribute("position", new THREE.BufferAttribute(wall1, 3));
    const w1_mat = new THREE.MeshBasicMaterial({ color: 0xff0000 });
    const w1_mesh = new THREE.Mesh(w1_geom, w1_mat);
    cmp.scene.add(w1_mesh);
    this.state.objects.push({ mesh: w1_mesh, name: "Wall1" });

    // prettier-ignore
    const wall2 = new Float32Array([

     -1.0, -1.0,  -1.0,
      1.0, -1.0,  -1.0,
      1.0,  1.0,  -1.0,

      1.0,  1.0,  -1.0,
     -1.0,  1.0,  -1.0,
     -1.0, -1.0,  -1.0
  ] );

    const w2_geom = new THREE.BufferGeometry();
    w2_geom.addAttribute("position", new THREE.BufferAttribute(wall2, 3));
    const w2_mat = new THREE.MeshBasicMaterial({ color: 0x00ff00 });
    const w2_mesh = new THREE.Mesh(w2_geom, w2_mat);
    cmp.scene.add(w2_mesh);
    this.state.objects.push({ mesh: w2_mesh, name: "Wall2" });

    // prettier-ignore
    const wall3 = new Float32Array([
        1.0,   1.0,  -1.0,
        1.0,  -1.0,  -1.0,
        1.0,  -1.0,   1.0,

        1.0,  1.0,  1.0,
        1.0,  1.0, -1.0,
        1.0, -1.0,  1.0
    ] );

    const w3_geom = new THREE.BufferGeometry();
    w3_geom.addAttribute("position", new THREE.BufferAttribute(wall3, 3));
    const w3_mat = new THREE.MeshBasicMaterial({ color: 0x0000ff });
    const w3_mesh = new THREE.Mesh(w3_geom, w3_mat);
    cmp.scene.add(w3_mesh);
    this.state.objects.push({ mesh: w3_mesh, name: "Wall3" });
    // prettier-ignore
    const wall4 = new Float32Array([
        -1.0,  -1.0,  -1.0,
        -1.0,   1.0,  -1.0,
        -1.0,  -1.0,   1.0,

        -1.0,  1.0, -1.0,
        -1.0,  1.0,  1.0,
        -1.0, -1.0,  1.0
    ] );

    const w4_geom = new THREE.BufferGeometry();
    w4_geom.addAttribute("position", new THREE.BufferAttribute(wall4, 3));
    const w4_mat = new THREE.MeshBasicMaterial({ color: 0xffff00 });
    const w4_mesh = new THREE.Mesh(w4_geom, w4_mat);
    cmp.scene.add(w4_mesh);
    this.state.objects.push({ mesh: w4_mesh, name: "Wall4" });

    // prettier-ignore
    const floor = new Float32Array([
         1.0, -1.0,  1.0,
         1.0, -1.0, -1.0,
        -1.0, -1.0,  1.0,

        -1.0, -1.0,  1.0,
         1.0, -1.0, -1.0,
        -1.0, -1.0, -1.0,
    ] );

    const textureSize = 1.0;
    // prettier-ignore
    const floorUVs = new Float32Array([
      textureSize, textureSize,
      textureSize, 0.0,
      0.0, textureSize,

      0.0, textureSize,
      textureSize, 0.0,
      0.0, 0.0,
    ]);

    console.log("Loading texture...");
    const texture = new THREE.TextureLoader().load(
      "textures/parquet-dark-golden-oak-hardwood-floor-pine-tree-colorful-seamless-texture-256x256.jpg",
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
    const floor_geom = new THREE.BufferGeometry();
    floor_geom.addAttribute("position", new THREE.BufferAttribute(floor, 3));
    floor_geom.addAttribute("uv", new THREE.BufferAttribute(floorUVs, 2));
    const floor_mat = new THREE.MeshBasicMaterial({
      map: texture
    });
    const floor_mesh = new THREE.Mesh(floor_geom, floor_mat);
    cmp.scene.add(floor_mesh);
    this.state.objects.push({ mesh: floor_mesh, name: "Floor" });

    const ogreloader = new OgreLoader();

    ogreloader.load(
      "modelxml",
      meshes => {
        console.log(`Model modelxml loaded: ${meshes.length} meshes`);
        meshes.forEach(m => {
          console.log(m);
          m.translateX(3.0);
          this.state.objects.push({ mesh: m, name: m.name });
          cmp.scene.add(m);
        });
      },
      progressEvent => {
        console.log("Progress loading model");
      },
      errorEvent => {
        console.log("Error loading model");
      }
    );

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
