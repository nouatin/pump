FROM node:latest
RUN mkdir -p /usr/src/pump
WORKDIR /usr/src/pump
COPY package.json /usr/src/pump/
RUN npm install
COPY . /usr/src/pump
EXPOSE 80
CMD ["npm", "start"]

