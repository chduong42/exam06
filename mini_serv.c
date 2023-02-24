/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exam06.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chduong <chduong@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/02/24 11:36:59 by chduong           #+#    #+#             */
/*   Updated: 2023/02/24 15:30:31 by chduong          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>

typedef struct s_client 
{
	int		id;
	char	msg[1024];
}			t_client;

void		error(char *msg)
{
	if (msg)
		write(2, msg, strlen(msg));
	else
		write(2, "Fatal error", 11);
	write(2, "\n", 1);
	exit(1);
}

fd_set 		readfds, active, writefds;
int			maxfd = 0, nextid = 0;
char 		buftoread[120000], buftowrite[120000];

void sendAll(int senderfd) 
{
	for (int fd = 0; fd <= maxfd; ++fd)
	{
		if (FD_ISSET(fd, &writefds) && fd != senderfd)
			send(fd, &buftowrite, strlen(buftowrite), 0);		
	}
}

int main(int ac, char **av)
{
	t_client				clients[1024];
	struct sockaddr_in 		servaddr;
	socklen_t 				len;
	
	bzero(clients, sizeof(clients));
	bzero(&servaddr, sizeof(servaddr));

	if (ac != 2)
		error("Wrong number of arguments");

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error(NULL);

	maxfd = sockfd;
	FD_ZERO(&active);
	FD_SET(sockfd, &active);
	
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(2130706433);
	servaddr.sin_port = htons(atoi(av[1]));

	if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 )
		error(NULL);
	if (listen(sockfd, 10) < 0 )
		error(NULL);

	while (1)
	{
		readfds = writefds = active;
		if (select(maxfd + 1, &readfds, &writefds, NULL, NULL) < 0)
			continue;
		for (int fd = 0; fd <= maxfd; ++fd)
		{
			if (FD_ISSET(fd, &readfds))
			{
				if (fd == sockfd)
				{
					int clientfd = accept(sockfd, (struct sockaddr*)&servaddr, &len);
					if (clientfd < 0)
						continue;
					maxfd = clientfd > maxfd ? clientfd : maxfd;
					clients[clientfd].id = nextid++;
					FD_SET(clientfd, &active);
					sprintf(buftowrite, "server: client %d has arrived\n", clients[clientfd].id);
					sendAll(clientfd);
					break;
				}
				else
				{
					int nbytes = recv(fd, buftoread, 70000, 0);
					if (nbytes <= 0)
					{
						sprintf(buftowrite, "server client %d has left\n", clients[fd].id);
						sendAll(fd);
						FD_CLR(fd, &active);
						close(fd);
						break;
					}
					else
					{
						for (int i = 0, j = strlen(clients[fd].msg); i < nbytes; ++i, ++j)
						{
							clients[fd].msg[j] = buftoread[i];
							if (clients[fd].msg[j] == '\n')
							{
								clients[fd].msg[j] ='\0';
								sprintf(buftowrite, "client %d: %s\n", clients[fd].id, clients[fd].msg);
								sendAll(fd);
								bzero(clients[fd].msg, strlen(clients[fd].msg));
								j = -1;
							}
						}
					}
					break;
				}
			}
		}
	}
}
