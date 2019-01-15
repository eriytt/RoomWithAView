package com.towersmatrix.rwav;

import java.io.IOException;

import android.util.Log;

import okhttp3.OkHttpClient;
import okhttp3.Request;
import okhttp3.Response;
import okhttp3.WebSocket;
import okhttp3.WebSocketListener;

import okio.ByteString;

public class HttpHandler
{
    private static final String HTTP_SERVER = "apoc.towersmatrix:5000";
    private OkHttpClient client;

    private String HttpURL(String endpoint) {
        return "http://" + HTTP_SERVER + endpoint;
    }

    private String WebsocketURL(String endpoint) {
        return "ws://" + HTTP_SERVER + endpoint;
    }

    private String logTag() {
        return "RoomWithAView(Java@HttpHandler)";
    }

    private final class EchoWebSocketListener extends WebSocketListener {
        private static final int NORMAL_CLOSURE_STATUS = 1000;

        @Override
        public void onOpen(WebSocket webSocket, Response response) {
            Log.i("RoomWithAView", "Connection open");
            // webSocket.send("Hello, it's SSaurel !");
            // webSocket.send("What's up ?");
            // webSocket.send(ByteString.decodeHex("deadbeef"));
            // //webSocket.close(NORMAL_CLOSURE_STATUS, "Goodbye !");
            // Log.i("RoomWithAView", "Messages sent");
        }

        @Override
        public void onMessage(WebSocket webSocket, String text) {
            Log.i(logTag(), "Receiving : " + text);
            if (text.equals("update")) {
                try {
                    Request modelRequest = new Request.Builder()
                        .url(HttpURL("/model"))
                        .build();

                    Log.i(logTag(), "GET /model");
                    Response response = client.newCall(modelRequest).execute();
                    try {
                        if (!response.isSuccessful()) throw new IOException("Unexpected code " + response);
                        okhttp3.ResponseBody responseBody = response.body();
                        byte[] bytes = responseBody.bytes();
                        Log.i(logTag(), "Got /model response ");
                        Native.UpdateModel(bytes);
                    } finally {
                        response.close();
                    }

                    Request metaRequest = new Request.Builder()
                        .url(HttpURL("/model/meta"))
                        .build();

                    Log.i(logTag(), "GET /model/meta");
                    response = client.newCall(metaRequest).execute();
                    try {
                        if (!response.isSuccessful()) throw new IOException("Unexpected code " + response);
                        Log.i(logTag(), "Got /model/meta response");
                        okhttp3.ResponseBody responseBody = response.body();
                        byte[] bytes = responseBody.bytes();
                        Native.UpdateMeta(bytes);
                    } finally {
                        response.close();
                    }
                } catch(IOException e) {}
            } else if (text.startsWith("update: ")) {
                String json = text.substring(8);
                Native.PartialUpdate(json);
            }
        }

        @Override
        public void onMessage(WebSocket webSocket, ByteString bytes) {
            Log.i(logTag(), "Receiving bytes : " + bytes.hex());
        }

        @Override
        public void onClosing(WebSocket webSocket, int code, String reason) {
            webSocket.close(NORMAL_CLOSURE_STATUS, null);
            Log.i(logTag(), "Closing : " + code + " / " + reason);
        }

        @Override
        public void onFailure(WebSocket webSocket, Throwable t, Response response) {
            Log.e(logTag(), "Error : " + t.getMessage());
        }
    }

    public HttpHandler() {
        Log.i(logTag(), "Starting handler");
        client = new OkHttpClient();
        //client.dispatcher().executorService().shutdown();
    }

    public void connectWebsocket() {
        Request request = new Request.Builder().url(WebsocketURL("/notifications")).build();
        EchoWebSocketListener listener = new EchoWebSocketListener();
        WebSocket ws = client.newWebSocket(request, listener);
    }

    public void download(final String uri, final DownloadFinished fini) {
        Request modelRequest = new Request.Builder()
            .url(HttpURL(uri))
            .build();

        client.newCall(modelRequest).enqueue(
            new okhttp3.Callback() {
                @Override public void onFailure(okhttp3.Call call, IOException e) {
                    e.printStackTrace();
                }

                @Override public void onResponse(okhttp3.Call call, Response response) throws IOException {
                    try {
                        okhttp3.ResponseBody responseBody = response.body();
                        if (!response.isSuccessful()) throw new IOException("Unexpected code " + response);
                        byte[] bytes = responseBody.bytes();

                        fini.finished(uri, bytes);
                    } finally {
                        response.close();
                    }
                }
            });
    }
}
