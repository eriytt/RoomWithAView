const loc = window.location;
const apiPort: number = 5000;

const apiRoot =
  loc.protocol + "//" + loc.hostname + (apiPort != -1 ? ":" + apiPort : "");

let headers = new Headers({
  "Content-Type": "application/json"
});

export const ApiClient = {
  post: function(path: string, data: object) {
    return fetch(apiRoot + path, {
      method: "POST",
      headers: headers,
      body: JSON.stringify(data)
    });
  },

  put: function(path: string, data: object) {
    return fetch(apiRoot + path, {
      method: "PUT",
      headers: headers,
      body: JSON.stringify(data)
    });
  },

  get: function(path: string) {
    return fetch(apiRoot + path, {
      method: "GET",
      headers: headers
    });
  }
};
