import React from "react";
import styled from "styled-components";

import { view } from "./app";

const ColorBox = styled.div`
  border: solid 2px black;
  width: 150px;
  height: 50px;
  background: ${(props: any) => props.color};
`;

type ColorMenuItemProps = {
  caption: string;
  subcaption: string;
  color: string;
};

class ColorMenuItem extends React.Component<ColorMenuItemProps, any> {
  click() {
    console.log("Setting color:", this.props.color);
    view.setObjectColor(this.props.color);
  }

  render() {
    return (
      <ColorBox
        color={this.props.color}
        onClick={() => {
          this.click();
        }}
      >
        <div>{this.props.caption}</div>
      </ColorBox>
    );
  }
}

export class ColorMenuComponent extends React.Component<{}, {}> {
  render() {
    return (
      <div>
        <div>Jotun Colors</div>
        <ColorMenuItem caption="DUNKELGRÅ" subcaption="4638" color="#667078" />
        <ColorMenuItem caption="SOMMARSOL" subcaption="10235" color="#E9CE8D" />
        <ColorMenuItem
          caption="Gustaviansk Blå"
          subcaption="4109"
          color="#9da9b2"
        />
        <ColorMenuItem caption="KAMELEONT" subcaption="4424" color="#9D9BA2" />
        <ColorMenuItem caption="PION" subcaption="3185" color="#B35668" />
        <ColorMenuItem caption="OKER" subcaption="1987" color="#CD965B" />
      </div>
    );
  }
}
