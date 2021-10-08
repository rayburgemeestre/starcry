const path = require('path');
const webpack = require('webpack');
const MonacoWebpackPlugin = require('monaco-editor-webpack-plugin');
const MiniCssExtractPlugin = require("mini-css-extract-plugin");
const {VueLoaderPlugin} = require('vue-loader');
const HtmlWebpackPlugin = require('html-webpack-plugin');

module.exports = {
  mode: 'production',
  entry: './websrc/index.js',
  output: {filename: 'main.js', path: path.resolve(__dirname, 'webroot')},
  module: {
    rules: [
      {
        test: /\.(sa|sc|c)ss$/,
        use: [
          MiniCssExtractPlugin.loader,
          "css-loader",
          "sass-loader",
        ],
      },
      {test: /\.vue$/, use: 'vue-loader'},
      {
        test: /\.ttf$/,
        use: ['file-loader'],
      }
    ]
  },
  plugins: [new MonacoWebpackPlugin(), new MiniCssExtractPlugin(), new VueLoaderPlugin()]
};
