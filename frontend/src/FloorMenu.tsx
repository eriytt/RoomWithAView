import React from "react";
import styled from "styled-components";

import { view } from "./app";

const FloorBox: any = styled.div`
  border: solid 2px black;
  width: 200px;
  height: 50px;
  background-image: url(${(props: any) => props.image});
`;

type FloorMenuItemProps = {
  caption: string;
  subcaption: string;
  image: string;
};

class FloorMenuItem extends React.Component<FloorMenuItemProps, any> {
  click() {
    view.setObjectTexture(this.props.image);
  }

  render() {
    return (
      <FloorBox
        image={this.props.image}
        onClick={() => {
          this.click();
        }}
      >
        {this.props.caption}
      </FloorBox>
    );
  }
}

export class FloorMenuComponent extends React.Component<{}, {}> {
  render() {
    return (
      <div>
        <div>Floors</div>
        <FloorMenuItem
          caption="Kährs Ulf Ek"
          subcaption=""
          image="textures/tragolv-kahrs-ulf-ek-1-stav.jpg"
        />
        <FloorMenuItem
          caption="Rappgo Ek Vitvax"
          subcaption=""
          image="textures/tragolv-rappgo-ek-vitvax.jpg"
        />
        <FloorMenuItem
          caption="Vaila grå"
          subcaption=""
          image="textures/DV_157x152_8775171_01_4c_DE_20130717224630.jpg"
        />
        <FloorMenuItem
          caption="Noss kakel grå"
          subcaption=""
          image="textures/DV_157x152_8352574_01_4c_SE_20170802162619.jpg"
        />
        <FloorMenuItem
          caption="Faray svart"
          subcaption=""
          image="textures/DV_157x152_5834529_01_4c_DE_20150622133957.jpg"
        />
      </div>
    );
  }
}
