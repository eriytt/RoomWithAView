import React from "react";
import * as THREE from "three";
import { Scene, Raycaster } from "three";
//import * as Math from "Math";

export type ThreeCanvasComponentProperties = {
  width: number;
  height: number;
  setup: (tcmp: ThreeCanvasComponent) => void;
};

export class ThreeCanvasComponent extends React.Component<
  ThreeCanvasComponentProperties,
  {}
> {
  scene = new THREE.Scene();
  renderer: THREE.WebGLRenderer | undefined = undefined;
  camera: THREE.PerspectiveCamera | undefined = undefined;
  r = 16.0;
  theta = Math.PI / 6.0;
  phi = 0.0;
  mouseX = this.props.width / 2.0;
  mouseY = this.props.height / 2.0;

  private raycaster = new Raycaster();
  picker:
    | ((cmp: ThreeCanvasComponent, hits: Array<THREE.Intersection>) => void)
    | null = null;
  private light = new THREE.PointLight(0xffffff);

  startGL() {
    const canvas = this.refs.canvas as HTMLCanvasElement;

    const params: THREE.WebGLRendererParameters = {
      canvas: canvas
    };

    console.log("Creating renderer");
    this.renderer = new THREE.WebGLRenderer(params);
    console.log("Configuring renderer");
    this.renderer.setSize(this.props.width, this.props.height);
    this.renderer.setClearColor(0x0, 0x1);

    this.camera = new THREE.PerspectiveCamera(
      45,
      this.props.width / this.props.height,
      1.0,
      100.0
    );
    /* this.camera.position.set(6, 7, 6);
       * this.camera.lookAt(new THREE.Vector3(0.0, 0.0, 0.0));*/
    this.setCamera();
    this.scene.add(this.camera);

    this.light.position.set(-100, 200, 100);
    this.scene.add(this.light);

    this.props.setup(this);
  }

  setCamera() {
    const x = this.r * Math.sin(this.theta) * Math.cos(this.phi);
    const z = this.r * Math.sin(this.theta) * Math.sin(this.phi);
    const y = this.r * Math.cos(this.theta);
    //const y = 6;
    this.camera!.position.set(x, y, z);
    this.camera!.lookAt(new THREE.Vector3(0.0, 0.0, 0.0));
  }

  componentDidMount() {
    const canvas = this.refs.canvas as HTMLCanvasElement;
    canvas.addEventListener("mousedown", event => {
      //console.log("mousedown", event);
      //{ target: <canvas>, buttons: 1, clientX: 366, clientY: 479, layerX: 366, layerY: 479 }
    });

    canvas.addEventListener("mouseup", event => {
      console.log("mouseup", event);
      //{ target: <canvas>, buttons: 1, clientX: 366, clientY: 479, layerX: 366, layerY: 479 }

      if (this.picker === null) return;

      const mouse = new THREE.Vector2();
      mouse.x = event.clientX / this.props.width * 2 - 1;
      mouse.y = -(event.clientY / this.props.height) * 2 + 1;

      this.raycaster.setFromCamera(mouse, this.camera!);

      // calculate objects intersecting the picking ray
      console.log(this.scene.children);
      const intersects = this.raycaster.intersectObjects(this.scene.children);
      this.picker(this, intersects);

      this.updateCanvas();
    });

    canvas.addEventListener("mousemove", event => {
      //{ target: <canvas>, buttons: 1, clientX: 696, clientY: 349, layerX: 696, layerY: 349 }
      if (event.buttons & 1) {
        const dx = event.clientX - this.mouseX;
        const dy = event.clientY - this.mouseY;
        this.phi += dx / (360 * 2 * Math.PI) * 10;
        this.theta -= dy / (360 * 2 * Math.PI) * 10;
        this.setCamera();
        this.updateCanvas();
      }
      this.mouseX = event.clientX;
      this.mouseY = event.clientY;
    });

    console.log("Starting GL");
    this.startGL();
    console.log("Updating canvas");
    this.updateCanvas();
  }

  componentDidUpdate() {
    this.updateCanvas();
  }

  updateCanvas() {
    this.renderer!.render(this.scene, this.camera!);
  }

  handleClick() {
    console.log("Clicked");
  }

  handleMouseMove() {
    console.log("Moved");
  }

  render() {
    return (
      <canvas
        ref="canvas"
        width={this.props.width}
        height={this.props.height}
      />
    );
  }
}
