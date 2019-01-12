import * as THREE from "three";
import { Face3 } from "three";

type OgreData = {
  meshes: THREE.Mesh[];
};

type Geometry = {
  vertices: THREE.Vector3[];
  normals: THREE.Vector3[] | undefined;
  uvs: UV[] | undefined;
};

type UV = number[];

type Vertex = {
  vertice?: THREE.Vector3;
  normal?: THREE.Vector3;
  uv: UV[];
};

type FaceData = {
  face: THREE.Face3;
  uvs: number[][];
};

const attrInt = (
  XMLNode: Element,
  attr: string,
  defaultValue?: number
): number => {
  if (!XMLNode || !XMLNode.getAttribute(attr)) return defaultValue || 0;
  return parseInt(XMLNode.getAttribute(attr)!);
};

const attrFloat = (XMLNode: Element, attr: string, defaultValue?: number) => {
  if (!XMLNode || !XMLNode.getAttribute(attr)) return defaultValue || 0.0;
  return parseFloat(XMLNode.getAttribute(attr)!);
};

const attrBool = (XMLNode: Element, attr: string, defaultValue?: boolean) => {
  if (!XMLNode || !XMLNode.getAttribute(attr)) return defaultValue || false;
  return XMLNode.getAttribute(attr)!.toLowerCase() == "true";
};

const attrVector3 = (XMLNode: Element) => {
  return new THREE.Vector3(
    attrFloat(XMLNode, "x"),
    attrFloat(XMLNode, "y"),
    attrFloat(XMLNode, "z")
  );
};

const attrU = (XMLNode: Element) => {
  return [attrFloat(XMLNode, "u")];
};

const attrUv = (XMLNode: Element) => {
  return [attrFloat(XMLNode, "u"), attrFloat(XMLNode, "v")];
};

const attrUvw = (XMLNode: Element) => {
  return [
    attrFloat(XMLNode, "u"),
    attrFloat(XMLNode, "v"),
    attrFloat(XMLNode, "w")
  ];
};

export class OgreLoader extends THREE.Loader implements THREE.AnyLoader {
  manager: THREE.LoadingManager;
  internalManager: THREE.LoadingManager;

  constructor(manager?: THREE.LoadingManager) {
    super();
    this.manager = manager !== undefined ? manager : new THREE.LoadingManager();
    this.internalManager = new THREE.LoadingManager();
  }

  load(url: string): Promise<THREE.Mesh[]> {
    return new Promise((resolve, reject) => {
      const loader = new THREE.FileLoader(this.manager);
      loader.load(
        url,
        (response: string) => {
          const parser = new DOMParser();
          const xml = parser.parseFromString(response, "text/xml");

          const data = this.parseOgreXml(xml);

          // if (data.scene) {
          //   scope.objectRoot.scene = data.scene;
          // }

          // if (data.meshes) {
          //   scope.objectRoot.meshes = data.meshes;
          // }

          // if (data.skeleton) {
          //   scope.objectRoot.skeleton = data.skeleton;
          // }

          // scope.internalManager.itemEnd(url);
          resolve(data.meshes);
        },
        (event: ProgressEvent) => {
          console.log("File loading progress:", event);
        },
        (event: ErrorEvent) => {
          reject(event);
        }
      );
    });
  }

  private parseOgreXml(xml: Document): OgreData {
    const name = xml.documentElement.nodeName;
    let meshes = undefined;

    switch (name) {
      // case "scene":
      //   data.scene = parseScene(xml.documentElement);
      //   break;

      case "mesh":
        meshes = this.parseMesh(xml.documentElement);
        break;

      // case "skeleton":
      //   data.skeleton = parseSkeleton(xml.documentElement);
      //   break;

      default:
        console.error(
          "THREE.XMLOgreLoader.parse(): Unknown node <" + name + ">"
        );
        console.error("	Cannot parse xml file");
        break;
    }

    return { meshes: meshes as THREE.Mesh[] };
  }

  private parseMesh(XMLNode: HTMLElement) {
    var geometry = new THREE.Geometry();
    const sharedGeometryNode = XMLNode.getElementsByTagName(
      "sharedgeometry"
    )[0];
    let sharedGeometry = undefined;
    let meshes: THREE.Mesh[] = [];
    //mesh, sharedGeometry;

    //geometry.merge = geomMerge;

    //if <sharedgeometry> exist
    if (sharedGeometryNode) {
      sharedGeometry = this.parseGeometry(sharedGeometryNode);
    }

    //if <submeshes> exist
    const submeshesNode = XMLNode.getElementsByTagName("submeshes")[0];
    if (submeshesNode) {
      meshes = this.parseSubmeshes(submeshesNode, sharedGeometry);
    }

    //if <submeshname> exist
    const submeshNamesNode = XMLNode.getElementsByTagName("submeshnames")[0];
    if (submeshNamesNode) {
      meshes = this.parseSubmeshnames(submeshNamesNode, meshes);
    }

    //merging all geometry for this mesh
    // for(var i = 0, il = meshes.length; i < il; i++){
    //   geometry.mergeMesh(meshes[i]);
    // }

    var material = new THREE.MeshBasicMaterial();

    // material.skinning = true;
    // material.morphTargets = true;
    //material.specular.setHSL( 0, 0, 0.1 );
    material.color.setHex(0xff0000);

    return meshes;
    //   .map(geometry => {
    //   var mesh = new THREE.Mesh(geometry, material);
    //   mesh.name = geometry.name;
    //   return mesh;
    // });
    // mesh = new THREE.SkinnedMesh(geometry, material);

    // //if <skeletonlink> exist
    // node = XMLNode.getElementsByTagName('skeletonlink')[0];
    // if(node){
    //   var internalLoader = new THREE.OgreMaxLoader(scope.manager),
    //       url = scope.path + node.getAttribute('name') + '.xml';

    //   scope.internalManager.itemStart(url);
    //   internalLoader.load(url, onFileLoaded, onFileProgress, onFileError);

    //   function onFileLoaded(data){
    //     //record data in objectRoot to use later (when all files are loaded)
    //     scope.objectRoot.skeletonFile = {
    //       skel: data.skeleton,
    //       anim: data.animations
    //     };

    //     scope.internalManager.itemEnd(url);
    //   };

    //   function onFileProgress(event){
    //   };

    //   //TODO
    //   function onFileError(event){
    //     scope.internalManager.itemError(url);
    //   };
    // }

    // return mesh;
  }

  // TODO: Support multiple vertexbuffers somehow
  private parseGeometry(XMLNode: Element): Geometry {
    //const count = attrInt(XMLNode, "vertexcount");

    const node = XMLNode.getElementsByTagName("vertexbuffer");

    const geometry = this.parseVertexbuffer(node[0]);

    if (geometry.vertices.length !== attrInt(XMLNode, "vertexcount")) {
      throw new Error(
        "vertices(" +
          geometry.vertices.length +
          ") and vertexcount(" +
          attrInt(XMLNode, "vertexcount") +
          ") should match"
      );
    }

    return geometry;
  }

  private parseVertexbuffer(XMLNode: Element): Geometry {
    const hasPositions = attrBool(XMLNode, "positions", false);
    const hasNormals = attrBool(XMLNode, "normals", false);
    const numTextureCoords = attrInt(XMLNode, "texture_coords", 0);
    const texture_coord_dimensions: number[] = [];

    const node = XMLNode.getElementsByTagName("vertex");

    for (var i = 0; i < numTextureCoords; i++) {
      const dimensionAttr = XMLNode.getAttribute(
        "texture_coord_dimensions_" + i
      );

      // TODO: support all dimenstions
      const dimension =
        dimensionAttr === null
          ? 2
          : parseInt(dimensionAttr.replace("float", ""));

      texture_coord_dimensions[i] = dimension;
    }

    let vertices: THREE.Vector3[] = [];
    let normals: THREE.Vector3[] = [];
    let uvs: UV[] = [];

    for (var i = 0, il = node.length; i < il; i++) {
      const vertex = this.parseVertex(
        node[i],
        hasPositions,
        hasNormals,
        numTextureCoords,
        texture_coord_dimensions
      );

      if (hasPositions)
        vertices = vertices.concat(vertex.vertice as THREE.Vector3);
      if (hasNormals) normals = normals.concat(vertex.normal as THREE.Vector3);
      uvs = uvs.concat(vertex.uv);
    }

    return {
      vertices: vertices,
      normals: hasNormals ? normals : undefined,
      uvs: numTextureCoords > 0 ? uvs : undefined
    };
  }

  private parseVertex(
    XMLNode: Element,
    positions: boolean,
    normals: boolean,
    texture_coords: number,
    texture_coord_dimensions: number[]
  ): Vertex {
    const uvs: UV[] = [];

    const getV3 = (name: string) => {
      const node = XMLNode.getElementsByTagName(name)[0];
      if (!node) throw new Error("Could not get element " + name);
      return attrVector3(node);
    };

    const vertice = positions ? getV3("position") : undefined;
    const normal = normals ? getV3("normal") : undefined;

    if (texture_coords > 0) {
      var node = XMLNode.getElementsByTagName("texcoord");

      for (var i = 0; i < texture_coords; i++) {
        switch (texture_coord_dimensions[i]) {
          case 1:
            uvs.push(attrU(node[i]));
            break;

          case 2:
            uvs.push(attrUv(node[i]));
            break;

          case 3:
            uvs.push(attrUvw(node[i]));
            break;
        }
      }
    }

    return { vertice: vertice, normal: normal, uv: uvs };
  }

  private parseSubmeshes(
    XMLNode: Element,
    sharedGeometry: Geometry | undefined
  ): THREE.Mesh[] {
    let meshes: THREE.Mesh[] = [];
    const node = XMLNode.getElementsByTagName("submesh");

    for (var i = 0, il = node.length; i < il; i++) {
      meshes.push(this.parseSubmesh(node[i], sharedGeometry));
    }

    return meshes;
  }

  parseSubmesh(XMLNode: Element, sharedGeometry: Geometry | undefined) {
    const normals: number[] = [];
    const uvs: number[] = [];
    const node = XMLNode.getElementsByTagName("geometry")[0];

    //mesh.userData.operationtype = XMLNode.getAttribute("operationtype");
    //mesh.userData.usesharedvertices = attrBool(XMLNode, "usesharedvertices");
    //mesh.userData.use32bitindexes = attrBool(XMLNode, "use32bitindexes");

    const usesharedvertices = attrBool(XMLNode, "usesharedvertices");

    if (usesharedvertices) {
      if (sharedGeometry == undefined)
        throw new Error(
          "Mesh uses shared geometry, but no shared geometry loaded"
        );
    } else if (!node) throw new Error("Mesh has no geometry");

    const geometry = usesharedvertices
      ? sharedGeometry!
      : this.parseGeometry(node);

    const geom = new THREE.BufferGeometry();
    geom.addAttribute(
      "position",
      new THREE.BufferAttribute(
        new Float32Array(geometry.vertices.flatMap(v => v.toArray())),
        3
      )
    );

    if (geometry.normals != undefined)
      geom.addAttribute(
        "normal",
        new THREE.BufferAttribute(
          new Float32Array(geometry.normals.flatMap(v => v.toArray())),
          3
        )
      );

    if (geometry.uvs != undefined)
      geom.addAttribute(
        "uv",
        new THREE.BufferAttribute(
          new Float32Array(geometry.uvs.flatMap(uv => uv)),
          2
        )
      );
    const mat = new THREE.MeshBasicMaterial({ color: 0xffff00 });

    const facesNode = XMLNode.getElementsByTagName("faces")[0];
    if (facesNode) {
      this.parseFaces(facesNode, geom, geometry.normals);
    }

    const mesh = new THREE.Mesh(geom, mat);

    // const node = XMLNode.getElementsByTagName("boneassignments")[0];
    // if (node) {
    //   parseBoneassignments(node, mesh.geometry);
    // }

    // if (XMLNode.getAttribute("operationtype") === "line_list") {
    //   mesh = new THREE.Line(mesh.geometry, mesh.material);
    // }

    // mesh.updateMorphTargets();

    return mesh;
  }

  private parseFaces(
    XMLNode: Element,
    geometry: THREE.BufferGeometry,
    normals: THREE.Vector3[] | undefined
  ) {
    const faceNode = XMLNode.getElementsByTagName("face");

    const faces = Array.from(faceNode).map((fn: Element) =>
      this.parseFace(fn, normals)
    );
    geometry.setIndex(
      new THREE.BufferAttribute(
        new Uint16Array(faces.flatMap(f => [f.a, f.b, f.c])),
        1 // Weird, doesn't work with 3
      )
    );
    // geometry.computeBoundingBox();
    // geometry.computeBoundingSphere();
    // geometry.computeFaceNormals();
  }

  private parseFace(XMLNode: Element, normals: THREE.Vector3[] | undefined) {
    const v1 = attrInt(XMLNode, "v1");
    const v2 = attrInt(XMLNode, "v2");
    const v3 = attrInt(XMLNode, "v3");
    const nrm =
      normals != undefined
        ? [normals[v1], normals[v2], normals[v3]]
        : undefined;

    return new THREE.Face3(v1, v2, v3, nrm);
  }

  private parseSubmeshnames(XMLNode: Element, meshes: THREE.Mesh[]) {
    const submeshNameNodes = XMLNode.getElementsByTagName("submeshname");

    console.log("Found submeshname:", submeshNameNodes);
    for (var i = 0, il = submeshNameNodes.length; i < il; i++) {
      meshes = this.parseSubmeshname(submeshNameNodes[i], meshes);
    }

    return meshes;
  }

  private parseSubmeshname(XMLNode: Element, meshes: THREE.Mesh[]) {
    const name = XMLNode.getAttribute("name");
    const index = attrInt(XMLNode, "index");
    meshes[index].name = name || "";

    return meshes;
  }

  onLoadStart = () => {
    return;
  };

  onLoadProgress = () => {
    return;
  };

  onLoadComplete = () => {
    return;
  };

  //load(url: string, onLoad: any, onProgress: any, onError: any) {}
}
