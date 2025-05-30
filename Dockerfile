FROM node:20

WORKDIR /app
COPY . .
RUN npm install
EXPOSE 25279
CMD ["npm", "start"]
